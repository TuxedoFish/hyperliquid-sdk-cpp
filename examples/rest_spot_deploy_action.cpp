#include "test_config.h"

#include <hyperliquid/rest/RestApi.h>
#include <hyperliquid/config/Config.h>
#include <spdlog/spdlog.h>

// TODO: coordinator will decide whether to run this live (creates permanent testnet state +
// costs deploy-auction gas) - may only get sad-path evidence.
//
// Demonstrates the full 5-step "create and launch a new spot token" (HIP-1/HIP-2) flow in
// sequence: registerToken2 -> genesis -> userGenesis -> registerSpot -> registerHyperliquidity.
// Each step depends on state created by the previous one (e.g. registerSpot needs the token index
// assigned by registerToken2), so in a real deployment you'd inspect spotDeployState() between
// steps to pick up the assigned token/spot indices rather than hardcoding them as done here.
//
// Deploying a token on testnet spends real (testnet) deploy-auction gas and creates permanent,
// irreversible on-chain state - this wallet is not expected to have an active gas auction bid,
// so registerToken2 is expected to be rejected with a real, informative error rather than
// silently succeeding, and the later steps are not reachable without a genuine deployment.
int main()
{
    auto wallet = loadWalletFromConfig();
    hyperliquid::setLogLevel(hyperliquid::LogLevel::Debug);

    hyperliquid::ApiConfig config;
    config.env = hyperliquid::Environment::Testnet;
    config.wallet = wallet;

    hyperliquid::RestApi api(config);

    spdlog::info("=== Step 1: spotDeployRegisterToken2 ===");
    hyperliquid::SpotDeployRegisterToken2Request registerToken2Req;
    registerToken2Req.spec.name = "TEST5";
    registerToken2Req.spec.szDecimals = 1;
    registerToken2Req.spec.weiDecimals = 7;
    registerToken2Req.maxGas = 1'000'000;
    registerToken2Req.fullName = "Example Test Token";

    auto registerToken2Resp = api.spotDeployRegisterToken2(registerToken2Req);
    spdlog::info("spotDeployRegisterToken2: status={} type={}", registerToken2Resp.status, registerToken2Resp.type);
    if (registerToken2Resp.error) spdlog::info("  Error: {}", *registerToken2Resp.error);

    // The token index assigned by registerToken2 would normally be read back via
    // spotDeployState(); hardcoded here purely to illustrate the shape of the later requests.
    int deployedTokenIndex = 40;

    spdlog::info("=== Step 2: spotDeployGenesis ===");
    hyperliquid::SpotDeployGenesisRequest genesisReq;
    genesisReq.token = deployedTokenIndex;
    genesisReq.maxSupply = 10000.0;
    genesisReq.noHyperliquidity = std::nullopt;

    auto genesisResp = api.spotDeployGenesis(genesisReq);
    spdlog::info("spotDeployGenesis: status={} type={}", genesisResp.status, genesisResp.type);
    if (genesisResp.error) spdlog::info("  Error: {}", *genesisResp.error);

    spdlog::info("=== Step 3: spotDeployUserGenesis ===");
    hyperliquid::SpotDeployUserGenesisRequest userGenesisReq;
    userGenesisReq.token = deployedTokenIndex;
    userGenesisReq.userAndWei = {{wallet.accountAddress, 100.0}};
    userGenesisReq.existingTokenAndWei = {};

    auto userGenesisResp = api.spotDeployUserGenesis(userGenesisReq);
    spdlog::info("spotDeployUserGenesis: status={} type={}", userGenesisResp.status, userGenesisResp.type);
    if (userGenesisResp.error) spdlog::info("  Error: {}", *userGenesisResp.error);

    spdlog::info("=== Step 4: spotDeployRegisterSpot ===");
    hyperliquid::SpotDeployRegisterSpotRequest registerSpotReq;
    registerSpotReq.baseToken = deployedTokenIndex;
    registerSpotReq.quoteToken = 0; // USDC

    auto registerSpotResp = api.spotDeployRegisterSpot(registerSpotReq);
    spdlog::info("spotDeployRegisterSpot: status={} type={}", registerSpotResp.status, registerSpotResp.type);
    if (registerSpotResp.error) spdlog::info("  Error: {}", *registerSpotResp.error);

    // The spot pair index assigned by registerSpot would normally be read back via
    // spotDeployState(); hardcoded here purely to illustrate the shape of the request.
    int deployedSpotIndex = 934;

    spdlog::info("=== Step 5: spotDeployRegisterHyperliquidity ===");
    hyperliquid::SpotDeployRegisterHyperliquidityRequest registerHyperliquidityReq;
    registerHyperliquidityReq.spot = deployedSpotIndex;
    registerHyperliquidityReq.startPx = 1.0;
    registerHyperliquidityReq.orderSz = 10.0;
    registerHyperliquidityReq.nOrders = 5;
    registerHyperliquidityReq.nSeededLevels = 3;

    auto registerHyperliquidityResp = api.spotDeployRegisterHyperliquidity(registerHyperliquidityReq);
    spdlog::info("spotDeployRegisterHyperliquidity: status={} type={}",
                 registerHyperliquidityResp.status, registerHyperliquidityResp.type);
    if (registerHyperliquidityResp.error) spdlog::info("  Error: {}", *registerHyperliquidityResp.error);

    return 0;
}
