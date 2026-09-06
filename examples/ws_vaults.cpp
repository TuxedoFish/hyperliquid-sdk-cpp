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

    const std::string vaultAddress = "0x1719884eb866cb12b2287399b15f7db5e7d775ea";

    uint64_t correlationId = 1;
    auto wait = []() { std::this_thread::sleep_for(std::chrono::milliseconds(300)); };

    ws.vaultDetails(vaultAddress, std::nullopt, correlationId++); wait();
    ws.userVaultEquities(userAddress, correlationId++); wait();
    ws.portfolio(userAddress, correlationId++); wait();
    ws.referral(userAddress, correlationId++); wait();
    ws.userRole(userAddress, correlationId++); wait();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    ws.stop();
    return 0;
}
