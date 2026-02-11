#include <filament_engine/core/log.h>

#include <cstdio>
#include <cstdarg>
#include <cstdlib>
#include <ctime>

namespace fe::detail {

static const char* levelToString(LogLevel level) {
    switch (level) {
        case LogLevel::Trace: return "TRACE";
        case LogLevel::Debug: return "DEBUG";
        case LogLevel::Info:  return "INFO ";
        case LogLevel::Warn:  return "WARN ";
        case LogLevel::Error: return "ERROR";
        case LogLevel::Fatal: return "FATAL";
    }
    return "?????";
}

static const char* levelToColor(LogLevel level) {
    switch (level) {
        case LogLevel::Trace: return "\033[90m";   // gray
        case LogLevel::Debug: return "\033[36m";    // cyan
        case LogLevel::Info:  return "\033[32m";    // green
        case LogLevel::Warn:  return "\033[33m";    // yellow
        case LogLevel::Error: return "\033[31m";    // red
        case LogLevel::Fatal: return "\033[1;31m";  // bold red
    }
    return "\033[0m";
}

void logMessage(LogLevel level, const char* file, int line, const char* fmt, ...) {
    // Extract filename from path
    const char* filename = file;
    for (const char* p = file; *p; ++p) {
        if (*p == '/' || *p == '\\') {
            filename = p + 1;
        }
    }

    const char* color = levelToColor(level);
    const char* levelStr = levelToString(level);

    fprintf(stderr, "%s[%s]%s %s:%d: ", color, levelStr, "\033[0m", filename, line);

    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);

    fprintf(stderr, "\n");

    if (level == LogLevel::Fatal) {
        std::abort();
    }
}

} // namespace fe::detail
