#include "Logger.hpp"
#include <Windows.h>
#include <algorithm>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <fmt/format.h>
#include <fmt/chrono.h>

namespace Broadsword::Services {

thread_local std::vector<LogContext> Logger::m_ContextStack;

Logger& Logger::Get()
{
    static Logger instance;
    return instance;
}

Logger::Logger() = default;

Logger::~Logger()
{
    Shutdown();
}

void Logger::Initialize()
{
    if (m_Running.load())
    {
        return; // Already initialized
    }

    m_Running.store(true);

    // Get current working directory (should be game's Binaries/Win64)
    char workingDir[MAX_PATH];
    GetCurrentDirectoryA(MAX_PATH, workingDir);

    // Create Logs directory in working directory
    std::string logsPath = std::string(workingDir) + "\\Logs";
    std::filesystem::create_directories(logsPath);

    // Open initial log file
    auto now = std::chrono::system_clock::now();
    auto t = std::chrono::system_clock::to_time_t(now);
    std::tm tm_time;
    localtime_s(&tm_time, &t);

    char timeBuffer[64];
    std::strftime(timeBuffer, sizeof(timeBuffer), "%Y-%m-%d_%H-%M-%S", &tm_time);

    m_CurrentLogFilePath = fmt::format("{}\\Broadsword_{}.log", logsPath, timeBuffer);

    {
        std::lock_guard<std::mutex> lock(m_FileMutex);
        m_CurrentLogFile.open(m_CurrentLogFilePath, std::ios::out | std::ios::app);
    }

    // Start async worker thread
    m_AsyncWriter = std::thread(&Logger::AsyncWriterThread, this);

    LOG_INFO("Broadsword Logger initialized");
    LOG_INFO("Log file: {}", m_CurrentLogFilePath);
}

void Logger::Shutdown()
{
    if (!m_Running.load())
    {
        return;
    }

    LOG_INFO("Shutting down Broadsword Logger");

    // Signal thread to stop
    m_Running.store(false);
    m_QueueCV.notify_all();

    // Wait for thread to finish
    if (m_AsyncWriter.joinable())
    {
        m_AsyncWriter.join();
    }

    // Close log file
    {
        std::lock_guard<std::mutex> lock(m_FileMutex);
        if (m_CurrentLogFile.is_open())
        {
            m_CurrentLogFile.close();
        }
    }
}

void Logger::SetOutputs(bool console, bool file, bool in_game)
{
    m_ConsoleOutput.store(console);
    m_FileOutput.store(file);
    m_InGameOutput.store(in_game);
}

void Logger::PushContext(std::string_view mod_name, std::string_view category)
{
    LogContext ctx;
    ctx.mod_name = mod_name;
    ctx.category = category;

    // Inherit tags from parent context
    if (!m_ContextStack.empty())
    {
        ctx.tags = m_ContextStack.back().tags;
    }

    m_ContextStack.push_back(std::move(ctx));
}

void Logger::PopContext()
{
    if (!m_ContextStack.empty())
    {
        m_ContextStack.pop_back();
    }
}

void Logger::AddTag(std::string_view key, std::string_view value)
{
    if (!m_ContextStack.empty())
    {
        m_ContextStack.back().tags[std::string(key)] = value;
    }
}

std::vector<LogEntry> Logger::QueryLogs(std::optional<LogLevel> min_level,
                                        std::optional<std::string> mod_filter,
                                        std::optional<uint64_t> frame_start,
                                        std::optional<uint64_t> frame_end,
                                        size_t max_results)
{
    std::lock_guard<std::mutex> lock(m_InGameMutex);

    std::vector<LogEntry> results;
    results.reserve((std::min)(max_results, m_InGameBuffer.size()));

    for (const auto& entry : m_InGameBuffer)
    {
        // Apply filters
        if (min_level.has_value() && entry.level < min_level.value())
        {
            continue;
        }

        if (mod_filter.has_value() && !mod_filter.value().empty())
        {
            if (entry.context.mod_name.find(mod_filter.value()) == std::string::npos)
            {
                continue;
            }
        }

        if (frame_start.has_value() && entry.frame_number < frame_start.value())
        {
            continue;
        }

        if (frame_end.has_value() && entry.frame_number > frame_end.value())
        {
            continue;
        }

        results.push_back(entry);

        if (results.size() >= max_results)
        {
            break;
        }
    }

    return results;
}

void Logger::EnqueueLog(LogEntry entry)
{
    {
        std::lock_guard<std::mutex> lock(m_QueueMutex);
        m_Queue.push(std::move(entry));
    }
    m_QueueCV.notify_one();
}

void Logger::AsyncWriterThread()
{
    while (m_Running.load())
    {
        std::unique_lock<std::mutex> lock(m_QueueMutex);

        // Wait for logs or shutdown signal
        m_QueueCV.wait(lock, [this] { return !m_Queue.empty() || !m_Running.load(); });

        // Process all pending logs
        while (!m_Queue.empty())
        {
            LogEntry entry = std::move(m_Queue.front());
            m_Queue.pop();
            lock.unlock();

            // Write to outputs
            if (m_ConsoleOutput.load())
            {
                WriteToConsole(entry);
            }

            if (m_FileOutput.load())
            {
                WriteToFile(entry);
            }

            if (m_InGameOutput.load())
            {
                WriteToInGame(entry);
            }

            lock.lock();
        }
    }

    // Flush remaining logs on shutdown
    std::unique_lock<std::mutex> lock(m_QueueMutex);
    while (!m_Queue.empty())
    {
        LogEntry entry = std::move(m_Queue.front());
        m_Queue.pop();
        lock.unlock();

        if (m_FileOutput.load())
        {
            WriteToFile(entry);
        }

        lock.lock();
    }
}

void Logger::WriteToConsole(const LogEntry& entry)
{
    // Format: [LEVEL] [Frame] [Mod] Message
    std::string output = fmt::format("[{}] [F:{}] [{}] {}\n",
                                     LogLevelToString(entry.level),
                                     entry.frame_number,
                                     entry.context.mod_name.empty() ? "Framework" : entry.context.mod_name,
                                     entry.message);

    // Write to stdout
    std::cout << output;
    std::cout.flush();
}

void Logger::WriteToFile(const LogEntry& entry)
{
    std::lock_guard<std::mutex> lock(m_FileMutex);

    if (!m_CurrentLogFile.is_open())
    {
        return;
    }

    // Write JSON entry
    m_CurrentLogFile << entry.ToJson().dump() << "\n";
    m_CurrentLogFile.flush();

    // Check if we need to rotate
    auto currentSize = m_CurrentLogFile.tellp();
    if (currentSize >= static_cast<std::streampos>(m_MaxFileSize))
    {
        RotateLogFiles();
    }
}

void Logger::WriteToInGame(const LogEntry& entry)
{
    std::lock_guard<std::mutex> lock(m_InGameMutex);

    m_InGameBuffer.push_back(entry);

    // Limit buffer size
    while (m_InGameBuffer.size() > m_MaxInGameEntries)
    {
        m_InGameBuffer.pop_front();
    }
}

void Logger::RotateLogFiles()
{
    // Close current file
    m_CurrentLogFile.close();

    // Get current working directory
    char workingDir[MAX_PATH];
    GetCurrentDirectoryA(MAX_PATH, workingDir);
    std::string logsPath = std::string(workingDir) + "\\Logs";

    // Delete oldest file if we have too many
    std::vector<std::filesystem::path> logFiles;
    if (std::filesystem::exists(logsPath))
    {
        for (const auto& entry : std::filesystem::directory_iterator(logsPath))
        {
            if (entry.path().extension() == ".log" &&
                entry.path().filename().string().starts_with("Broadsword_"))
            {
                logFiles.push_back(entry.path());
            }
        }
    }

    // Sort by modification time
    std::sort(logFiles.begin(), logFiles.end(), [](const auto& a, const auto& b) {
        return std::filesystem::last_write_time(a) < std::filesystem::last_write_time(b);
    });

    // Delete oldest files if we exceed max count
    while (logFiles.size() >= static_cast<size_t>(m_MaxFiles))
    {
        std::filesystem::remove(logFiles.front());
        logFiles.erase(logFiles.begin());
    }

    // Open new file
    auto now = std::chrono::system_clock::now();
    auto t = std::chrono::system_clock::to_time_t(now);
    std::tm tm_time;
    localtime_s(&tm_time, &t);

    char timeBuffer[64];
    std::strftime(timeBuffer, sizeof(timeBuffer), "%Y-%m-%d_%H-%M-%S", &tm_time);

    m_CurrentLogFilePath = fmt::format("{}\\Broadsword_{}.log", logsPath, timeBuffer);
    m_CurrentLogFile.open(m_CurrentLogFilePath, std::ios::out | std::ios::app);
}

std::string Logger::GetThreadName()
{
    DWORD threadId = GetCurrentThreadId();

    // Game thread is usually the main thread
    static DWORD gameThreadId = 0;
    if (gameThreadId == 0)
    {
        gameThreadId = threadId;
        return "GameThread";
    }

    if (threadId == gameThreadId)
    {
        return "GameThread";
    }

    return fmt::format("Thread{}", threadId);
}

// ScopedLog implementation
Logger::ScopedLog::ScopedLog(Logger& logger,
                             std::string_view operation,
                             SourceLocation loc)
    : m_Logger(logger), m_Start(std::chrono::steady_clock::now())
{
    m_Entry.timestamp = std::chrono::system_clock::now();
    m_Entry.frame_number = m_Logger.m_CurrentFrame;
    m_Entry.level = LogLevel::Debug;
    m_Entry.thread_id = GetCurrentThreadId();
    m_Entry.thread_name = m_Logger.GetThreadName();
    m_Entry.source = loc;

    if (!m_Logger.m_ContextStack.empty())
    {
        m_Entry.context = m_Logger.m_ContextStack.back();
    }

    m_Entry.message = fmt::format("Operation: {}", operation);
}

Logger::ScopedLog::~ScopedLog()
{
    auto end = std::chrono::steady_clock::now();
    m_Entry.duration =
        std::chrono::duration_cast<std::chrono::microseconds>(end - m_Start);

    m_Logger.EnqueueLog(std::move(m_Entry));
}

Logger::ScopedLog Logger::ScopedOperation(std::string_view operation,
                                          const char* file,
                                          int line,
                                          const char* function)
{
    SourceLocation loc;
    loc.file = file;
    loc.line = line;
    loc.function = function;

    return ScopedLog(*this, operation, loc);
}

} // namespace Broadsword::Services
