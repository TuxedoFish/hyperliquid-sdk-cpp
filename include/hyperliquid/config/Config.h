#pragma once

#include <set>

#include "../types/RequestTypes.h"

namespace hyperliquid
{
    enum class LogLevel { Trace, Debug, Info, Warn, Error, Critical, Off };

    void setLogLevel(LogLevel level);

    // The signing identity used for authenticated requests. accountAddress is the account being
    // acted on (0x...); privateKey is the key that actually signs (your main wallet's key, or an
    // approved agent/API wallet's key acting on accountAddress's behalf - see approveAgent).
    struct Wallet
    {
        std::string accountAddress;
        std::string privateKey;
    };

    struct ApiConfig
    {
        Environment env = Environment::Testnet;
        // Leave unset to only use unauthenticated /info-style endpoints; any authenticated
        // request (an /exchange action, or an authenticated /info call) throws without one.
        std::optional<Wallet> wallet;
        // Builder-deployed perp dexes this config should also fetch metadata for when building
        // the internal symbol map (see skipBuildingSymbolMap) - leave empty for the main dex only.
        std::set<std::string> dexes;
        // RestApi/WebsocketApi build a coin/asset -> id symbol map on construction (needed to
        // resolve order/leverage/margin request fields) by calling meta()/spotMeta() over the
        // network. Set true to skip that call - only safe if you never call an endpoint that
        // needs asset ids resolved by name.
        bool skipBuildingSymbolMap = false;
        // Default vault/subaccount address used when a per-call vaultAddress isn't given - see
        // Signing::prepareBodyForType for which action types this fallback does and doesn't apply to.
        std::optional<std::string> vaultAddress;
    };
}
