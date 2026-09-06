#include "test_config.h"

#include <hyperliquid/rest/RestApi.h>
#include <hyperliquid/config/Config.h>
#include <spdlog/spdlog.h>

int main()
{
    auto wallet = loadWalletFromConfig();
    hyperliquid::setLogLevel(hyperliquid::LogLevel::Debug);

    hyperliquid::ApiConfig config;
    config.env = hyperliquid::Environment::Testnet;
    config.wallet = wallet;

    hyperliquid::RestApi api(config);

    hyperliquid::UserSetAbstractionRequest req;
    req.user = wallet.accountAddress;
    req.abstraction = hyperliquid::AbstractionMode::Disabled;

    // The exchange rejects this with a real, informative error if the account has open
    // positions, open orders, or active TWAP orders when switching away from UnifiedAccount -
    // that's expected exchange behavior, not an SDK error.
    auto resp = api.userSetAbstraction(req);
    spdlog::info("userSetAbstraction: status={} type={}", resp.status, resp.type);
    if (resp.error)
        spdlog::info("  Error: {}", *resp.error);

    return 0;
}
