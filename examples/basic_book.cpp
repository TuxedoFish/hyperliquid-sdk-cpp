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
    // hyperliquid::WebsocketListener
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
    void onL2BookLevel(const hyperliquid::L2BookUpdate& book, const hyperliquid::PriceLevel& level) override {
        spdlog::info("{} [L2Book] {} {} {} @ {} ({})", book.time, book.coin,
                     level.side == hyperliquid::Side::Bid ? "BID" : "ASK",
                     level.sz, level.px, level.n);
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
    hyperliquid::WebsocketApi websocket(hyperliquid::Environment::Mainnet, printer);

    spdlog::info("Subscribing to BTC l2Book + bbo + trades for 5 seconds...");
    websocket.start();

    websocket.subscribe(hyperliquid::SubscriptionType::L2Book, {{"coin", "BTC"}});
    websocket.subscribe(hyperliquid::SubscriptionType::Bbo, {{"coin", "BTC"}});
    websocket.subscribe(hyperliquid::SubscriptionType::Trades, {{"coin", "BTC"}});

    std::this_thread::sleep_for(std::chrono::seconds(5));
    websocket.stop();

    return 0;
}
