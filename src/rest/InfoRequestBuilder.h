#pragma once

#include <optional>
#include <string>

#include <nlohmann/json.hpp>

#include "hyperliquid/types/RequestTypes.h"

namespace hyperliquid {

class InfoRequestBuilder {
public:
    static nlohmann::json spotMeta();
    static nlohmann::json meta(const std::optional<std::string>& dex = std::nullopt);
    static nlohmann::json perpDexs();
};

} // namespace hyperliquid
