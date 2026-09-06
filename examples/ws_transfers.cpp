#include <chrono>
#include <thread>

#include <spdlog/spdlog.h>

#include "test_config.h"
#include "hyperliquid/websocket/WebsocketApi.h"
#include "hyperliquid/websocket/WebsocketApiListener.h"

class PostResponseLogger : public hyperliquid::WebsocketApiListener
{
public:
    void onPostResponse(const std::string& rawJson, hyperliquid::RestEndpointType type,
                        std::optional<uint64_t> correlationId) override
    {
        spdlog::info("[post response] type={} correlationId={} payload={}",
                     hyperliquid::toString(type), correlationId.value_or(0), rawJson);
    }

    void onConnected() override
    {
        spdlog::info("Connected");
    }
};

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

    const std::string destination = "0x5e9ee1089755c3435139848e47e6635505d5a13a";
    const std::string tokenId = "USDC:0xeb62eee3685fc4c43992febcd9e75443";

    uint64_t correlationId = 1;
    auto wait = []() { std::this_thread::sleep_for(std::chrono::seconds(3)); };

    hyperliquid::UsdClassTransferRequest classTransferReq;
    classTransferReq.amount = 1;
    classTransferReq.toPerp = false;
    ws.usdClassTransfer(classTransferReq, correlationId++); wait();

    hyperliquid::UsdSendRequest usdSendReq;
    usdSendReq.destination = destination;
    usdSendReq.amount = 1;
    ws.usdSend(usdSendReq, correlationId++); wait();

    hyperliquid::SpotSendRequest spotSendReq;
    spotSendReq.destination = destination;
    spotSendReq.token = tokenId;
    spotSendReq.amount = 1;
    ws.spotSend(spotSendReq, correlationId++); wait();

    hyperliquid::SendAssetRequest sendAssetReq;
    sendAssetReq.destination = destination;
    sendAssetReq.sourceDex = "";
    sendAssetReq.destinationDex = "";
    sendAssetReq.token = tokenId;
    sendAssetReq.amount = 1;
    sendAssetReq.fromSubAccount = "";
    ws.sendAsset(sendAssetReq, correlationId++); wait();

    hyperliquid::Withdraw3Request withdrawReq;
    withdrawReq.destination = destination;
    withdrawReq.amount = 2; // withdrawals below ~$2 are typically rejected by the bridge
    ws.withdraw3(withdrawReq, correlationId++); wait();
    ws.stop();
    return 0;
}
