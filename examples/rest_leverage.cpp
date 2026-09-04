#include "test_config.h"

#include <hyperliquid/rest/RestApi.h>
#include <hyperliquid/config/Config.h>
#include <spdlog/spdlog.h>

void logSimpleResponse(const char* action, const hyperliquid::SimpleResponse& resp)
{
    spdlog::info("{}: status={} type={}", action, resp.status, resp.type);
    if (resp.error)
        spdlog::info("  Error: {}", *resp.error);
}

int main()
{
    auto wallet = loadWalletFromConfig();
    hyperliquid::setLogLevel(hyperliquid::LogLevel::Debug);

    hyperliquid::ApiConfig config;
    config.env = hyperliquid::Environment::Testnet;
    config.wallet = wallet;

    hyperliquid::RestApi api(config);

    // NOTE: these are signed, state-changing exchange actions - they persist on the
    // account (even on testnet) once executed. This example is intentionally not run
    // automatically; a human should run it deliberately.

    spdlog::info("=== Update leverage ===");

    hyperliquid::UpdateLeverageRequest leverageReq;
    leverageReq.asset = "ETH";
    leverageReq.isCross = true;
    leverageReq.leverage = 10;

    auto leverageResp = api.updateLeverage(leverageReq);
    logSimpleResponse("Update leverage (cross, 10x)", leverageResp);

    spdlog::info("=== Update isolated margin ===");

    hyperliquid::UpdateIsolatedMarginRequest marginReq;
    marginReq.asset = "ETH";
    marginReq.isBuy = true;
    marginReq.ntli = 1000000; // add $1.00 (6 decimals: 1,000,000 == 1 USD)

    auto marginResp = api.updateIsolatedMargin(marginReq);
    logSimpleResponse("Update isolated margin (+$1.00)", marginResp);

    return 0;
}
