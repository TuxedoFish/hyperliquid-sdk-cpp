#pragma once

#include <optional>
#include <string>

#include <nlohmann/json.hpp>

#include "hyperliquid/types/RequestTypes.h"

namespace hyperliquid {

class InfoRequestBuilder {
public:
    static nlohmann::ordered_json spotMeta();
    static nlohmann::ordered_json meta(const std::optional<std::string>& dex = std::nullopt);
    static nlohmann::ordered_json outcomeMeta();
    static nlohmann::ordered_json perpDexs();
};

} // namespace hyperliquid
