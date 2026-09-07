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

// WebSocket equivalent of examples/rest_perp_deploy_action.cpp - same registerAsset2 request
// (see that file for the marginTableId note) against the same real, live "test" HIP-3 dex,
// expected to be rejected the same way.
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

    hyperliquid::PerpDeployRegisterAsset2Request req;
    req.maxGas = std::nullopt;
    req.assetRequest.coin = "SDKTEST";
    req.assetRequest.szDecimals = 2;
    req.assetRequest.oraclePx = 1.0;
    req.assetRequest.marginTableId = 10;
    req.assetRequest.marginMode = hyperliquid::PerpMarginMode::Normal;
    req.dex = "test";
    req.schema = std::nullopt;
    ws.perpDeployRegisterAsset2(req, 1);
    std::this_thread::sleep_for(std::chrono::seconds(3));

    ws.stop();
    return 0;
}
