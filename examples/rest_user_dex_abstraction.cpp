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

    // Deprecated in favor of userSetAbstraction (which replaces the bool enabled/disabled toggle
    // with the 3-way UnifiedAccount/Isolated/PortfolioMargin AbstractionMode enum) - kept here so
    // the SDK's coverage of this still-live endpoint has real evidence behind it.
    hyperliquid::UserDexAbstractionRequest req;
    req.user = wallet.accountAddress;
    req.enabled = false;

    auto disableResp = api.userDexAbstraction(req);
    spdlog::info("userDexAbstraction(enabled=false): status={} type={}", disableResp.status, disableResp.type);
    if (disableResp.error)
        spdlog::info("  Error: {}", *disableResp.error);

    // Confirmed live: re-enabling consistently rejects with "Abstraction transition not allowed"
    // once the account is already in UnifiedAccount mode (via userSetAbstraction) - the same
    // rejection pattern observed on the agentSetAbstraction investigation. Expected exchange
    // behavior, not an SDK error.
    req.enabled = true;
    auto enableResp = api.userDexAbstraction(req);
    spdlog::info("userDexAbstraction(enabled=true): status={} type={}", enableResp.status, enableResp.type);
    if (enableResp.error)
        spdlog::info("  Error: {}", *enableResp.error);

    return 0;
}
