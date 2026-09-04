#pragma once

#include <set>

#include "../types/RequestTypes.h"

namespace hyperliquid
{
    enum class LogLevel { Trace, Debug, Info, Warn, Error, Critical, Off };

    void setLogLevel(LogLevel level);

    struct Wallet
    {
        std::string accountAddress;
        std::string privateKey;
    };

    struct ApiConfig
    {
        Environment env;
        std::optional<Wallet> wallet;
        std::set<std::string> dexes;
        bool skipBuildingSymbolMap;
        std::optional<std::string> vaultAddress;
    };
}
