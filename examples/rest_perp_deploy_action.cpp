#include "test_config.h"

#include <hyperliquid/rest/RestApi.h>
#include <hyperliquid/config/Config.h>
#include <spdlog/spdlog.h>

// Demonstrates registerAsset2 - deploying a new HIP-3 perp asset onto an existing dex. "test" is
// a real, live HIP-3 dex on testnet with a deployer address that isn't this wallet, so this is
// expected to be rejected with a real, informative error rather than silently succeeding.
// marginTableId must reference a real, existing margin table (e.g. 10) - an arbitrary id like 0
// makes the whole request fail JSON deserialization server-side with an opaque error, rather
// than a normal business-logic rejection.
int main()
{
    auto wallet = loadWalletFromConfig();
    hyperliquid::setLogLevel(hyperliquid::LogLevel::Debug);

    hyperliquid::ApiConfig config;
    config.env = hyperliquid::Environment::Testnet;
    config.wallet = wallet;

    hyperliquid::RestApi api(config);

    hyperliquid::PerpDeployRegisterAsset2Request req;
    req.maxGas = std::nullopt; // use the current deploy auction price
    req.assetRequest.coin = "SDKTEST";
    req.assetRequest.szDecimals = 2;
    req.assetRequest.oraclePx = 1.0;
    req.assetRequest.marginTableId = 10;
    req.assetRequest.marginMode = hyperliquid::PerpMarginMode::Normal;
    req.dex = "test";
    req.schema = std::nullopt;

    auto resp = api.perpDeployRegisterAsset2(req);
    spdlog::info("perpDeployRegisterAsset2: status={} type={}", resp.status, resp.type);
    if (resp.error)
        spdlog::info("  Error: {}", *resp.error);

    return 0;
}
