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

    hyperliquid::VaultTransferRequest vaultReq;
    vaultReq.vaultAddress = "0xa15099a30bbf2e68942d6f4c43d70d04faeab0a0"; // Hyperliquidity Provider (HLP)
    vaultReq.isDeposit = true;
    vaultReq.usd = 6.0; // vault minimum deposit is $5
    ws.vaultTransfer(vaultReq, correlationId++); wait();
    ws.stop();
    return 0;
}
