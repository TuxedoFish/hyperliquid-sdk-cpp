#include "test_config.h"

#include <hyperliquid/rest/RestApi.h>
#include <hyperliquid/config/Config.h>
#include <spdlog/spdlog.h>

// userDexAbstractionState() is named to avoid colliding with the *exchange* action
// RestApi::userDexAbstraction(const UserDexAbstractionRequest&), which shares the same wire
// name but flips the toggle rather than reading it. Both queries here are unauthenticated.
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
