#include <chrono>
#include <thread>

#include <spdlog/spdlog.h>

#include "test_config.h"
#include "hyperliquid/websocket/WebsocketApi.h"
#include "hyperliquid/websocket/WebsocketApiListener.h"

class PostResponseLogger : public hyperliquid::WebsocketApiListener
{
public:
    void onPostResponse(hyperliquid::RestEndpointType type, const hyperliquid::SimpleResponse& resp,
                        std::optional<uint64_t> correlationId) override
    {
        spdlog::info("[{}] correlationId={} status={} type={}",
                     hyperliquid::toString(type), correlationId.value_or(0), resp.status, resp.type);
        if (resp.error)
            spdlog::info("  Error: {}", *resp.error);
    }

    void onConnected() override
    {
        spdlog::info("Connected");
    }
};

// WebSocket equivalent of examples/rest_spot_deploy_action.cpp - same 5-step flow, same
// hardcoded illustrative indices, same expectation that registerToken2 is rejected since this
// wallet has no active deploy-auction bid.
int main()
{
    auto wallet = loadWalletFromConfig();
    hyperliquid::setLogLevel(hyperliquid::LogLevel::Debug);

    hyperliquid::ApiConfig config;
    config.env = hyperliquid::Environment::Testnet;
    config.wallet = wallet;

    PostResponseLogger logger;
    hyperliquid::WebsocketApi ws(config, logger);

    ws.start();
    std::this_thread::sleep_for(std::chrono::seconds(2));

    hyperliquid::SpotDeployRegisterToken2Request registerToken2Req;
    registerToken2Req.spec.name = "TEST5";
    registerToken2Req.spec.szDecimals = 1;
    registerToken2Req.spec.weiDecimals = 7;
    registerToken2Req.maxGas = 1'000'000;
    registerToken2Req.fullName = "Example Test Token";
    ws.spotDeployRegisterToken2(registerToken2Req, 1);
    std::this_thread::sleep_for(std::chrono::seconds(3));

    int deployedTokenIndex = 40;

    hyperliquid::SpotDeployGenesisRequest genesisReq;
    genesisReq.token = deployedTokenIndex;
    genesisReq.maxSupply = 10000.0;
    genesisReq.noHyperliquidity = std::nullopt;
    ws.spotDeployGenesis(genesisReq, 2);
    std::this_thread::sleep_for(std::chrono::seconds(3));

    hyperliquid::SpotDeployUserGenesisRequest userGenesisReq;
    userGenesisReq.token = deployedTokenIndex;
    userGenesisReq.userAndWei = {{wallet.accountAddress, 100.0}};
    userGenesisReq.existingTokenAndWei = {};
    ws.spotDeployUserGenesis(userGenesisReq, 3);
    std::this_thread::sleep_for(std::chrono::seconds(3));

    hyperliquid::SpotDeployRegisterSpotRequest registerSpotReq;
    registerSpotReq.baseToken = deployedTokenIndex;
    registerSpotReq.quoteToken = 0; // USDC
    ws.spotDeployRegisterSpot(registerSpotReq, 4);
    std::this_thread::sleep_for(std::chrono::seconds(3));

    int deployedSpotIndex = 934;

    hyperliquid::SpotDeployRegisterHyperliquidityRequest registerHyperliquidityReq;
    registerHyperliquidityReq.spot = deployedSpotIndex;
    registerHyperliquidityReq.startPx = 1.0;
    registerHyperliquidityReq.orderSz = 10.0;
    registerHyperliquidityReq.nOrders = 5;
    registerHyperliquidityReq.nSeededLevels = 3;
    ws.spotDeployRegisterHyperliquidity(registerHyperliquidityReq, 5);
    std::this_thread::sleep_for(std::chrono::seconds(3));

    ws.stop();
    return 0;
}
