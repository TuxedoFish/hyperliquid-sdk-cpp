#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace hyperliquid
{
    struct AssetMeta
    {
        std::string name;
        int szDecimals;
        int maxLeverage;
    };

    struct MetaResponse
    {
        std::vector<AssetMeta> universe;
    };
}
