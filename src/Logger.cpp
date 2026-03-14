#include "hyperliquid/Logger.h"
#include "Logger.h"

namespace hyperliquid {

static spdlog::level::level_enum toSpdlogLevel(LogLevel level)
{
    switch (level)
    {
    case LogLevel::Trace:    return spdlog::level::trace;
    case LogLevel::Debug:    return spdlog::level::debug;
    case LogLevel::Info:     return spdlog::level::info;
    case LogLevel::Warn:     return spdlog::level::warn;
    case LogLevel::Error:    return spdlog::level::err;
    case LogLevel::Critical: return spdlog::level::critical;
    case LogLevel::Off:      return spdlog::level::off;
    default:                 return spdlog::level::info;
    }
}

void setLogLevel(LogLevel level)
{
    getLogger()->set_level(toSpdlogLevel(level));
}

} // namespace hyperliquid
