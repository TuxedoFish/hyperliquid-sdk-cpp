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

    // Supply a small, reversible amount of token 0 (USDC) to the borrow/lend pool, then withdraw
    // it straight back out - same pair as examples/rest_borrow_lend_action.cpp.
    hyperliquid::BorrowLendRequest supplyReq;
    supplyReq.operation = hyperliquid::BorrowLendOperation::Supply;
    supplyReq.token = 0;
    supplyReq.amount = 1.0;
    ws.borrowLend(supplyReq, 1);
    std::this_thread::sleep_for(std::chrono::seconds(3));

    hyperliquid::BorrowLendRequest withdrawReq;
    withdrawReq.operation = hyperliquid::BorrowLendOperation::Withdraw;
    withdrawReq.token = 0;
    withdrawReq.amount = 1.0;
    ws.borrowLend(withdrawReq, 2);
    std::this_thread::sleep_for(std::chrono::seconds(3));

    ws.stop();
    return 0;
}
