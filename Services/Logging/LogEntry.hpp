#pragma once

#include <chrono>
#include <string>
#include <unordered_map>
#include <nlohmann/json.hpp>

namespace Broadsword::Services {

enum class LogLevel {
    Trace,
    Debug,
    Info,
    Warning,
    Error,
    Critical,
};

inline const char* LogLevelToString(LogLevel level)
{
    switch (level)
    {
    case LogLevel::Trace:
        return "TRACE";
    case LogLevel::Debug:
        return "DEBUG";
    case LogLevel::Info:
        return "INFO";
    case LogLevel::Warning:
        return "WARNING";
    case LogLevel::Error:
        return "ERROR";
    case LogLevel::Critical:
        return "CRITICAL";
    default:
        return "UNKNOWN";
    }
}

inline LogLevel LogLevelFromString(const std::string& level)
{
    if (level == "TRACE")
        return LogLevel::Trace;
    if (level == "DEBUG")
        return LogLevel::Debug;
    if (level == "INFO")
        return LogLevel::Info;
    if (level == "WARNING" || level == "WARN")
        return LogLevel::Warning;
    if (level == "ERROR")
        return LogLevel::Error;
    if (level == "CRITICAL" || level == "FATAL")
        return LogLevel::Critical;
    return LogLevel::Info;
}

struct SourceLocation {
    std::string file;
    int line = 0;
    std::string function;
};

struct LogContext {
    std::string mod_name;
    std::string category;
    std::unordered_map<std::string, std::string> tags;
};

struct LogEntry {
    std::chrono::system_clock::time_point timestamp;
    uint64_t frame_number = 0;
    LogLevel level = LogLevel::Info;
    uint32_t thread_id = 0;
    std::string thread_name;

    SourceLocation source;
    LogContext context;

    std::string message;
    nlohmann::json data;

    std::chrono::microseconds duration{0};
    size_t memory_usage_bytes = 0;

    nlohmann::json ToJson() const
    {
        auto t = std::chrono::system_clock::to_time_t(timestamp);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(timestamp.time_since_epoch()) %
                  1000;

        std::tm tm_time;
        localtime_s(&tm_time, &t);

        char timeBuffer[64];
        std::strftime(timeBuffer, sizeof(timeBuffer), "%Y-%m-%dT%H:%M:%S", &tm_time);

        nlohmann::json j = {{"timestamp", std::string(timeBuffer) + "." + std::to_string(ms.count())},
                            {"frame", frame_number},
                            {"level", LogLevelToString(level)},
                            {"thread", thread_id},
                            {"thread_name", thread_name},
                            {"source",
                             {
                                 {"file", source.file},
                                 {"line", source.line},
                                 {"function", source.function},
                             }},
                            {"context",
                             {
                                 {"mod", context.mod_name},
                                 {"category", context.category},
                                 {"tags", context.tags},
                             }},
                            {"message", message}};

        if (!data.is_null())
        {
            j["data"] = data;
        }

        if (duration.count() > 0)
        {
            j["duration_us"] = duration.count();
        }

        if (memory_usage_bytes > 0)
        {
            j["memory_mb"] = memory_usage_bytes / (1024.0 * 1024.0);
        }

        return j;
    }
};

} // namespace Broadsword::Services
