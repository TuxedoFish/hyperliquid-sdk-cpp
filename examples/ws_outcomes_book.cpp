#include <thread>
#include <chrono>

#include <spdlog/spdlog.h>

#include "hyperliquid/rest/RestApi.h"
#include "hyperliquid/rest/RestApiMessageParser.h"
#include "hyperliquid/websocket/WebsocketApi.h"
#include "hyperliquid/websocket/WebsocketApiListener.h"
#include "hyperliquid/websocket/WebsocketMessageHandler.h"
#include "hyperliquid/websocket/WebsocketMessageParser.h"

class OutcomeBookListener : public hyperliquid::WebsocketMessageHandler,
                            public hyperliquid::WebsocketApiListener {
public:
    void onMessage(const std::string& message) override {
        messageParser.crack(message, *this);
    }

    void onConnected() override {
        spdlog::info("Connected");
    }

    void onDisconnected(bool hasError, const std::string& errMsg) override {
        spdlog::info("Disconnected: hasError={} errMsg={}", hasError, errMsg);
    }

    void onL2Book(const hyperliquid::L2BookSnapshot& snapshot) override {
        spdlog::info("{} [L2Book] {} bids={} asks={}", snapshot.time, snapshot.coin,
                     snapshot.numBids, snapshot.numAsks);
        for (uint8_t i = 0; i < snapshot.numBids; i++) {
            const auto& lvl = snapshot.bids[i];
            spdlog::info("  BID {} @ {} ({})", lvl.sz, lvl.px, lvl.n);
        }
        for (uint8_t i = 0; i < snapshot.numAsks; i++) {
            const auto& lvl = snapshot.asks[i];
            spdlog::info("  ASK {} @ {} ({})", lvl.sz, lvl.px, lvl.n);
        }
    }

    void onTrade(const hyperliquid::Trade& trade) override {
        spdlog::info("{} [TRADE] {} {} {} @ {}", trade.time, trade.coin, trade.side, trade.sz, trade.px);
    }

private:
    hyperliquid::WebsocketMessageParser messageParser;
};

int main() {
    hyperliquid::setLogLevel(hyperliquid::LogLevel::Debug);

    hyperliquid::ApiConfig config;
    config.env = hyperliquid::Environment::Mainnet;

    // Fetch outcomeMeta and find BTC 1d market
    hyperliquid::RestApi restApi(config);

    auto outcomeMeta = restApi.outcomeMeta();
    spdlog::info("Fetched {} outcomes, searching for underlying=BTC period=1d...", outcomeMeta.outcomes.size());

    int outcomeIndex = -1;
    for (const auto& outcome : outcomeMeta.outcomes) {
        if (outcome.description.underlying == "BTC" && outcome.description.period == "1d") {
            outcomeIndex = outcome.outcome;
            spdlog::info("Found BTC 1d outcome: index={} class={} expiry={} targetPrice={}",
                         outcome.outcome, outcome.description.outcomeClass,
                         std::chrono::system_clock::to_time_t(outcome.description.expiry), outcome.description.targetPrice);
            break;
        }
    }

    if (outcomeIndex < 0) {
        spdlog::error("No BTC 1d outcome found");
        return 1;
    }

    // Subscribe to book for "Yes" side (side=0)
    std::string coin = hyperliquid::outcomeCoin(outcomeIndex, 0);
    spdlog::info("Subscribing to L2Book + trades for coin={}", coin);

    OutcomeBookListener listener;
    hyperliquid::WebsocketApi websocket(config, listener);

    websocket.start();
    websocket.subscribe(hyperliquid::SubscriptionType::L2Book, {{"coin", coin}});
    websocket.subscribe(hyperliquid::SubscriptionType::Trades, {{"coin", coin}});

    std::this_thread::sleep_for(std::chrono::seconds(5));
    websocket.stop();

    return 0;
}
