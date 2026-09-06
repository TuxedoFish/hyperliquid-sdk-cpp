#include "test_config.h"

#include <hyperliquid/rest/RestApi.h>
#include <hyperliquid/config/Config.h>
#include <spdlog/spdlog.h>

// TODO: coordinator will decide whether to run this live (creates permanent testnet state +
// costs deploy-auction gas) - may only get sad-path evidence.
//
// perpDeploy is a large multi-variant L1 action (16 sub-actions sharing "type": "perpDeploy");
// only registerAsset2 - deploying a new HIP-3 perp asset, optionally creating a new dex - is
// wired up here. The other 15 variants (setOracle, setFundingMultipliers, haltTrading, margin
// table config, fee config, sub-deployers, etc.) are out of scope for this issue.

int main()
{
    auto wallet = loadWalletFromConfig();
    hyperliquid::setLogLevel(hyperliquid::LogLevel::Debug);

    hyperliquid::ApiConfig config;
    config.env = hyperliquid::Environment::Testnet;
    config.wallet = wallet;

    hyperliquid::RestApi api(config);

    // "test" is a real, live HIP-3 dex on testnet (see examples/json/rest/parsePerpDexs/perp_dexs.json)
    // with a deployer address that isn't this wallet, so registering a new asset against it is
    // expected to be rejected with a real, informative error rather than silently succeeding.
    // schema is left unset (null) since we're adding an asset to an EXISTING dex, not creating
    // a new one.
    hyperliquid::PerpDeployRegisterAsset2Request req;
    req.maxGas = std::nullopt; // use the current deploy auction price
    req.assetRequest.coin = "SDKTEST";
    req.assetRequest.szDecimals = 2;
    req.assetRequest.oraclePx = 1.0;
    req.assetRequest.marginTableId = 0;
    req.assetRequest.marginMode = hyperliquid::PerpMarginMode::Normal;
    req.dex = "test";
    req.schema = std::nullopt;

    auto resp = api.perpDeployRegisterAsset2(req);
    spdlog::info("perpDeployRegisterAsset2: status={} type={}", resp.status, resp.type);
    if (resp.error)
        spdlog::info("  Error: {}", *resp.error);

    return 0;
}
