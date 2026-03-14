#pragma once

namespace hyperliquid {

enum class LogLevel { Trace, Debug, Info, Warn, Error, Critical, Off };

void setLogLevel(LogLevel level);

} // namespace hyperliquid
