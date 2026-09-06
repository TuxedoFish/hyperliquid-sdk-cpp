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

    // Only the HIP-3 DEX's designated backstop liquidator address may deposit/withdraw here - this
    // wallet isn't one, so a real, informative rejection is expected, same as
    // rest_hip3_liquidator_transfer.cpp. ntl must be a multiple of 1000 quote tokens (1e-6 units).
    hyperliquid::Hip3LiquidatorTransferRequest hip3Req;
    hip3Req.dex = "test";
    hip3Req.ntl = 1'000'000'000ULL;
    hip3Req.isDeposit = true;
    ws.hip3LiquidatorTransfer(hip3Req, correlationId++); wait();
    ws.stop();
    return 0;
}
