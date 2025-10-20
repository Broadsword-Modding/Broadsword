#pragma once

#include "LogEntry.hpp"
#include <Windows.h>
#include <atomic>
#include <condition_variable>
#include <deque>
#include <fstream>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <thread>
#include <vector>
#include <fmt/format.h>

namespace Broadsword::Services {

class Logger {
public:
    static Logger& Get();

    Logger();
    ~Logger();

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    void Initialize();
    void Shutdown();

    // Main logging interface with fmt formatting
    template <typename... Args>
    void Log(LogLevel level,
             const char* file,
             int line,
             const char* function,
             const char* format,
             Args&&... args)
    {
        if (level < m_MinLevel)
        {
            return;
        }

        LogEntry entry;
        entry.timestamp = std::chrono::system_clock::now();
        entry.frame_number = m_CurrentFrame;
        entry.level = level;
        entry.thread_id = GetCurrentThreadId();
        entry.thread_name = GetThreadName();

        entry.source.file = file;
        entry.source.line = line;
        entry.source.function = function;

        if (!m_ContextStack.empty())
        {
            entry.context = m_ContextStack.back();
        }

        if constexpr (sizeof...(Args) > 0)
        {
            entry.message = fmt::vformat(format, fmt::make_format_args(args...));
        }
        else
        {
            entry.message = format;
        }

        EnqueueLog(std::move(entry));
    }

    // Convenience methods
    template <typename... Args>
    void Trace(const char* file,
               int line,
               const char* function,
               const char* format,
               Args&&... args)
    {
        Log(LogLevel::Trace, file, line, function, format, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void Debug(const char* file,
               int line,
               const char* function,
               const char* format,
               Args&&... args)
    {
        Log(LogLevel::Debug, file, line, function, format, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void Info(const char* file,
              int line,
              const char* function,
              const char* format,
              Args&&... args)
    {
        Log(LogLevel::Info, file, line, function, format, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void Warn(const char* file,
              int line,
              const char* function,
              const char* format,
              Args&&... args)
    {
        Log(LogLevel::Warning, file, line, function, format, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void Error(const char* file,
               int line,
               const char* function,
               const char* format,
               Args&&... args)
    {
        Log(LogLevel::Error, file, line, function, format, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void Critical(const char* file,
                  int line,
                  const char* function,
                  const char* format,
                  Args&&... args)
    {
        Log(LogLevel::Critical, file, line, function, format, std::forward<Args>(args)...);
    }

    // Scoped logging for performance tracking
    class ScopedLog {
    public:
        ScopedLog(Logger& logger, std::string_view operation, SourceLocation loc);
        ~ScopedLog();

        template <typename T>
        void AddData(std::string_view key, T&& value)
        {
            m_Entry.data[std::string(key)] = std::forward<T>(value);
        }

    private:
        Logger& m_Logger;
        std::chrono::steady_clock::time_point m_Start;
        LogEntry m_Entry;
    };

    ScopedLog ScopedOperation(std::string_view operation,
                              const char* file,
                              int line,
                              const char* function);

    // Context management
    void PushContext(std::string_view mod_name, std::string_view category = "");
    void PopContext();
    void AddTag(std::string_view key, std::string_view value);

    // Configuration
    void SetMinLevel(LogLevel level) { m_MinLevel = level; }
    void SetOutputs(bool console, bool file, bool in_game);
    void SetMaxFileSize(size_t bytes) { m_MaxFileSize = bytes; }
    void SetMaxFiles(int count) { m_MaxFiles = count; }

    // Frame tracking
    void SetCurrentFrame(uint64_t frame) { m_CurrentFrame = frame; }
    uint64_t GetCurrentFrame() const { return m_CurrentFrame; }

    // Query logs (for in-game console)
    std::vector<LogEntry> QueryLogs(std::optional<LogLevel> min_level = {},
                                    std::optional<std::string> mod_filter = {},
                                    std::optional<uint64_t> frame_start = {},
                                    std::optional<uint64_t> frame_end = {},
                                    size_t max_results = 1000);

    // Force flush all pending logs to disk
    void Flush();

private:
    void EnqueueLog(LogEntry entry);
    void AsyncWriterThread();

    void WriteToConsole(const LogEntry& entry);
    void WriteToFile(const LogEntry& entry);
    void WriteToInGame(const LogEntry& entry);

    void RotateLogFiles();
    std::string GetThreadName();

    // Async queue
    std::queue<LogEntry> m_Queue;
    std::mutex m_QueueMutex;
    std::condition_variable m_QueueCV;
    std::thread m_AsyncWriter;
    std::atomic<bool> m_Running{false};

    // Configuration
    std::atomic<LogLevel> m_MinLevel{LogLevel::Info};
    std::atomic<bool> m_ConsoleOutput{true};
    std::atomic<bool> m_FileOutput{true};
    std::atomic<bool> m_InGameOutput{true};

    // File output
    std::ofstream m_CurrentLogFile;
    std::string m_CurrentLogFilePath;
    size_t m_MaxFileSize = 50 * 1024 * 1024; // 50 MB
    int m_MaxFiles = 5;
    std::mutex m_FileMutex;

    // In-game buffer
    std::deque<LogEntry> m_InGameBuffer;
    size_t m_MaxInGameEntries = 10000;
    std::mutex m_InGameMutex;

    // Context stack (thread-local)
    static thread_local std::vector<LogContext> m_ContextStack;

    // Frame tracking
    std::atomic<uint64_t> m_CurrentFrame{0};
};

} // namespace Broadsword::Services

// Convenience macros for automatic source location
#define LOG_TRACE(...) \
    ::Broadsword::Services::Logger::Get().Trace(__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#define LOG_DEBUG(...) \
    ::Broadsword::Services::Logger::Get().Debug(__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#define LOG_INFO(...) \
    ::Broadsword::Services::Logger::Get().Info(__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#define LOG_WARN(...) \
    ::Broadsword::Services::Logger::Get().Warn(__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#define LOG_ERROR(...) \
    ::Broadsword::Services::Logger::Get().Error(__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#define LOG_CRITICAL(...) \
    ::Broadsword::Services::Logger::Get().Critical(__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)

#define LOG_SCOPED(operation) \
    auto CONCAT(_scoped_log_, __LINE__) = \
        ::Broadsword::Services::Logger::Get().ScopedOperation(operation, __FILE__, __LINE__, __FUNCTION__)
