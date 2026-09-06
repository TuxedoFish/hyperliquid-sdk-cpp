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

    const uint64_t now = static_cast<uint64_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count());
    const uint64_t oneHourAgo = now - 3600ULL * 1000ULL;

    uint64_t correlationId = 1;
    auto wait = []() { std::this_thread::sleep_for(std::chrono::milliseconds(300)); };

    ws.l2Book("BTC", std::nullopt, std::nullopt, correlationId++); wait();
    ws.candleSnapshot("BTC", "15m", oneHourAgo, now, correlationId++); wait();
    ws.allMids(std::nullopt, correlationId++); wait();
    ws.openOrders(userAddress, std::nullopt, correlationId++); wait();
    // Plausible-but-unlikely-to-exist oid: exercises the request/response wiring even though
    // the server is expected to reply with a "not found"-style response rather than an order.
    ws.orderStatus(userAddress, hyperliquid::OrderId{12345678ULL}, correlationId++); wait();
    ws.userFills(userAddress, std::nullopt, std::nullopt, correlationId++); wait();
    ws.userFillsByTime(userAddress, oneHourAgo, std::nullopt, std::nullopt, std::nullopt, correlationId++); wait();
    ws.clearinghouseState(userAddress, std::nullopt, correlationId++); wait();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    ws.stop();
    return 0;
}
