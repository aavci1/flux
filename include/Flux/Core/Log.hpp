#pragma once

#include <cstdio>
#include <cstdarg>

namespace flux {

enum class LogLevel { Trace, Debug, Info, Warn, Error, Off };

namespace detail {

inline LogLevel& minLogLevel() {
    static LogLevel level =
#ifdef NDEBUG
        LogLevel::Info;
#else
        LogLevel::Debug;
#endif
    return level;
}

inline void log(LogLevel level, const char* fmt, ...) {
    if (level < minLogLevel()) return;

    const char* prefix = "";
    switch (level) {
        case LogLevel::Trace: prefix = "[TRACE] "; break;
        case LogLevel::Debug: prefix = "[DEBUG] "; break;
        case LogLevel::Info:  prefix = "[INFO]  "; break;
        case LogLevel::Warn:  prefix = "[WARN]  "; break;
        case LogLevel::Error: prefix = "[ERROR] "; break;
        case LogLevel::Off:   return;
    }

    std::fprintf(stderr, "%s", prefix);
    va_list args;
    va_start(args, fmt);
    std::vfprintf(stderr, fmt, args);
    va_end(args);
    std::fprintf(stderr, "\n");
}

} // namespace detail

inline void setLogLevel(LogLevel level) { detail::minLogLevel() = level; }

} // namespace flux

#ifdef NDEBUG
#define FLUX_LOG_TRACE(...)
#define FLUX_LOG_DEBUG(...)
#else
#define FLUX_LOG_TRACE(...) ::flux::detail::log(::flux::LogLevel::Trace, __VA_ARGS__)
#define FLUX_LOG_DEBUG(...) ::flux::detail::log(::flux::LogLevel::Debug, __VA_ARGS__)
#endif

#define FLUX_LOG_INFO(...)  ::flux::detail::log(::flux::LogLevel::Info, __VA_ARGS__)
#define FLUX_LOG_WARN(...)  ::flux::detail::log(::flux::LogLevel::Warn, __VA_ARGS__)
#define FLUX_LOG_ERROR(...) ::flux::detail::log(::flux::LogLevel::Error, __VA_ARGS__)
