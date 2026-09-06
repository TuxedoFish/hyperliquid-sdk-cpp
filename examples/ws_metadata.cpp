#include <chrono>
#include <thread>

#include <spdlog/spdlog.h>

#include "test_config.h"
#include "hyperliquid/types/RequestTypes.h"
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

    void onDisconnected(bool hasError, const std::string& errMsg) override
    {
        spdlog::info("Disconnected: hasError={} errMsg={}", hasError, errMsg);
    }
};

int main()
{
    std::string userAddress = loadWalletFromConfig().accountAddress;
    hyperliquid::setLogLevel(hyperliquid::LogLevel::Info);

    hyperliquid::ApiConfig config;
    config.env = hyperliquid::Environment::Testnet;
    config.skipBuildingSymbolMap = true;

    PostResponseLogger logger;
    hyperliquid::WebsocketApi ws(config, logger);

    ws.start();
    std::this_thread::sleep_for(std::chrono::seconds(2));

    const std::string builderAddress = "0x1719884eb866cb12b2287399b15f7db5e7d775ea";

    uint64_t correlationId = 1;
    auto wait = []() { std::this_thread::sleep_for(std::chrono::milliseconds(300)); };

    ws.metaAndAssetCtxs(std::nullopt, correlationId++); wait();
    ws.spotMetaAndAssetCtxs(correlationId++); wait();
    ws.spotClearinghouseState(userAddress, std::nullopt, correlationId++); wait();
    ws.frontendOpenOrders(userAddress, std::nullopt, correlationId++); wait();
    ws.historicalOrders(userAddress, correlationId++); wait();
    ws.userTwapSliceFills(userAddress, correlationId++); wait();
    ws.subAccounts(userAddress, correlationId++); wait();
    ws.userFees(userAddress, correlationId++); wait();
    ws.maxBuilderFee(userAddress, builderAddress, correlationId++); wait();
    ws.approvedBuilders(userAddress, correlationId++); wait();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    ws.stop();
    return 0;
}
