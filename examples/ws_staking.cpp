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

    ws.delegations(wallet.accountAddress, correlationId++); wait();
    ws.delegatorSummary(wallet.accountAddress, correlationId++); wait();
    ws.delegatorHistory(wallet.accountAddress, correlationId++); wait();
    ws.delegatorRewards(wallet.accountAddress, correlationId++); wait();

    ws.cDeposit(3ULL * 100000000ULL, correlationId++); wait(); // 3 HYPE

    hyperliquid::TokenDelegateRequest delegateReq;
    delegateReq.validator = "0x0000472d488d33b7329ca53bfcc3918961d55f8e"; // "Puffer Node"
    delegateReq.wei = 2ULL * 100000000ULL; // 2 HYPE
    delegateReq.isUndelegate = false;
    ws.tokenDelegate(delegateReq, correlationId++); wait();

    // Delegations have a lock-up before they can be undelegated, and cWithdraw has a separate
    // multi-day unstaking queue - expect a lock-up/queue rejection here, same as rest_staking.cpp.
    ws.cWithdraw(1ULL * 100000000ULL, correlationId++); wait();

    ws.stop();
    return 0;
}
