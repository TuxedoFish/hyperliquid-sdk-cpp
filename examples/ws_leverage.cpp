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


    uint64_t correlationId = 1;
    auto wait = []() { std::this_thread::sleep_for(std::chrono::seconds(3)); };

    hyperliquid::UpdateLeverageRequest leverageReq;
    leverageReq.asset = "ETH";
    leverageReq.isCross = false;
    leverageReq.leverage = 10;
    ws.updateLeverage(leverageReq, correlationId++); wait();

    // updateIsolatedMargin needs an existing isolated position to act on - rest_leverage.cpp opens
    // one first via placeOrder. This example doesn't, so a rejection here is expected; it still
    // confirms the request reaches the correct business-logic check.
    hyperliquid::UpdateIsolatedMarginRequest marginReq;
    marginReq.asset = "ETH";
    marginReq.isBuy = true;
    marginReq.ntli = 1000000; // $1.00 (6 decimals)
    ws.updateIsolatedMargin(marginReq, correlationId++); wait();
    ws.stop();
    return 0;
}
