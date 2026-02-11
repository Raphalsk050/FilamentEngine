#pragma once

#include <cstdio>
#include <cstdarg>

namespace fe {

// Log severity levels
enum class LogLevel {
    Trace,
    Debug,
    Info,
    Warn,
    Error,
    Fatal
};

namespace detail {

void logMessage(LogLevel level, const char* file, int line, const char* fmt, ...);

} // namespace detail

} // namespace fe

// Logging macros with file/line context
#define FE_LOG_TRACE(fmt, ...) ::fe::detail::logMessage(::fe::LogLevel::Trace, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define FE_LOG_DEBUG(fmt, ...) ::fe::detail::logMessage(::fe::LogLevel::Debug, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define FE_LOG_INFO(fmt, ...)  ::fe::detail::logMessage(::fe::LogLevel::Info,  __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define FE_LOG_WARN(fmt, ...)  ::fe::detail::logMessage(::fe::LogLevel::Warn,  __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define FE_LOG_ERROR(fmt, ...) ::fe::detail::logMessage(::fe::LogLevel::Error, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define FE_LOG_FATAL(fmt, ...) ::fe::detail::logMessage(::fe::LogLevel::Fatal, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
