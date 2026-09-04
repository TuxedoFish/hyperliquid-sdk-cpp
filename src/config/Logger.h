#pragma once

#include <memory>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace hyperliquid {

inline std::shared_ptr<spdlog::logger> getLogger() {
    auto logger = spdlog::get("hyperliquid");
    if (!logger) {
        try {
            logger = spdlog::stdout_color_mt("hyperliquid");
        } catch (const spdlog::spdlog_ex&) {
            logger = spdlog::get("hyperliquid");
        }
    }
    return logger;
}

} // namespace hyperliquid
