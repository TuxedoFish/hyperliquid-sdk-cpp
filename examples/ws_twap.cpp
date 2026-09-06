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

    hyperliquid::TwapOrderRequest twapReq;
    twapReq.asset = "ETH";
    twapReq.isBuy = true;
    twapReq.size = 0.05;
    twapReq.reduceOnly = false;
    twapReq.minutes = 10;
    twapReq.randomize = true;
    ws.twapOrder(twapReq, correlationId++); wait();

    // Real twapId chaining would require parsing the twapOrder response above (see ws_orders.cpp
    // for that RestApiMessageParser pattern) - out of scope for this wiring smoke test, so this
    // uses a placeholder id and expects a "twap not found"-style rejection rather than a real
    // cancel.
    hyperliquid::TwapCancelRequest cancelReq;
    cancelReq.asset = "ETH";
    cancelReq.twapId = 0;
    ws.twapCancel(cancelReq, correlationId++); wait();
    ws.stop();
    return 0;
}
