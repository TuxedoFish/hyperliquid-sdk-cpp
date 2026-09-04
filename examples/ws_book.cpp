#include <hyperliquid/types/ResponseTypes.h>
#include <thread>
#include <chrono>

#include <spdlog/spdlog.h>

#include "hyperliquid/websocket/WebsocketApi.h"
#include "hyperliquid/websocket/WebsocketApiListener.h"
#include "hyperliquid/websocket/WebsocketMessageHandler.h"
#include "hyperliquid/websocket/WebsocketMessageParser.h"

class BookPrinter : public hyperliquid::WebsocketMessageHandler, public hyperliquid::WebsocketApiListener {
public:
    void onMessage(const std::string& message) override {
        messageParser.crack(message, *this);
    }

    void onConnected() override {
        spdlog::info("Connected");
    }

    void onDisconnected(bool hasError, const std::string& errMsg) override {
        spdlog::info("Disconnected");
    }

    // hyperliquid::WSMessageHandler
    void onL2Book(const hyperliquid::L2BookSnapshot& snapshot) override {
        spdlog::info("{} [L2Book] {} bids={} asks={}", snapshot.time, snapshot.coin,
                     snapshot.numBids, snapshot.numAsks);
        for (uint8_t i = 0; i < snapshot.numBids; i++)
        {
            const auto& lvl = snapshot.bids[i];
            spdlog::info("  BID {} @ {} ({})", lvl.sz, lvl.px, lvl.n);
        }
        for (uint8_t i = 0; i < snapshot.numAsks; i++)
        {
            const auto& lvl = snapshot.asks[i];
            spdlog::info("  ASK {} @ {} ({})", lvl.sz, lvl.px, lvl.n);
        }
    }

    void onBbo(const hyperliquid::BboUpdate& bbo) override {
        std::string msg = fmt::format("{} [BBO] {}", bbo.time, bbo.coin);
        if (bbo.hasBid)
            msg += fmt::format(" BID {} @ {}", bbo.bid.sz, bbo.bid.px);
        if (bbo.hasAsk)
            msg += fmt::format(" ASK {} @ {}", bbo.ask.sz, bbo.ask.px);
        spdlog::info(msg);
    }

    void onTrade(const hyperliquid::Trade& trade) override {
        spdlog::info("{} [TRADE] {} {} {} @ {}", trade.time, trade.coin, trade.side, trade.sz, trade.px);
    }

private:
    hyperliquid::WebsocketMessageParser messageParser;
};

int main() {
    BookPrinter printer;
    hyperliquid::ApiConfig config;
    config.env = hyperliquid::Environment::Mainnet;
    hyperliquid::WebsocketApi websocket(config, printer);

    spdlog::info("Subscribing to BTC l2Book + bbo + trades for 5 seconds...");
    websocket.start();

    websocket.subscribe(hyperliquid::SubscriptionType::L2Book, {{"coin", "BTC"}});
    websocket.subscribe(hyperliquid::SubscriptionType::Bbo, {{"coin", "BTC"}});
    websocket.subscribe(hyperliquid::SubscriptionType::Trades, {{"coin", "BTC"}});

    std::this_thread::sleep_for(std::chrono::seconds(5));
    websocket.stop();

    return 0;
}
