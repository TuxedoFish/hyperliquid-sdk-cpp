#include "test_config.h"

#include <hyperliquid/rest/RestApi.h>
#include <hyperliquid/config/Config.h>
#include <spdlog/spdlog.h>

// Demonstrates the two read-only /info account/dex abstraction queries added for issue #48:
//
//   - userDexAbstraction (info): "is dex abstraction toggled on for this user" - a bare
//     true/false/null response. Not to be confused with the *exchange* action of the same wire
//     name (RestApi::userDexAbstraction(const UserDexAbstractionRequest&)), which flips the
//     toggle rather than reading it - the info-side method here is named
//     userDexAbstractionState() to avoid the collision.
//   - userAbstraction (info): the account's current abstraction mode - a bare string, one of
//     "unifiedAccount"/"portfolioMargin"/"disabled"/"default".
//
// Both are unauthenticated reads - no signing/wallet needed, only a user address to query.

int main()
{
    // Only the account address is used below - the private key is never read or logged.
    std::string userAddress = loadWalletFromConfig().accountAddress;

    hyperliquid::setLogLevel(hyperliquid::LogLevel::Info);

    hyperliquid::ApiConfig config;
    config.env = hyperliquid::Environment::Testnet;
    config.skipBuildingSymbolMap = true;

    hyperliquid::RestApi api(config);

    if (userAddress.empty())
    {
        spdlog::warn("No wallet configured - skipping live queries.");
        return 0;
    }

    spdlog::info("=== userDexAbstractionState(\"{}\") ===", userAddress);
    auto dexAbstraction = api.userDexAbstractionState(userAddress);
    spdlog::info("  enabled={}",
                 dexAbstraction.enabled ? (*dexAbstraction.enabled ? "true" : "false") : "null");

    spdlog::info("=== userAbstraction(\"{}\") ===", userAddress);
    auto abstraction = api.userAbstraction(userAddress);
    spdlog::info("  state={}", hyperliquid::toString(abstraction.state));

    return 0;
}
