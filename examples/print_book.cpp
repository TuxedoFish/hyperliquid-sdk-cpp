#include <hyperliquid/websocket/MarketData.h>
#include <hyperliquid/websocket/WSMessageHandler.h>
#include <hyperliquid/websocket/WSMessageParser.h>
#include <hyperliquid/websocket/WebsocketListener.h>
#include <hyperliquid/types/ResponseTypes.h>
#include <iostream>
#include <thread>
#include <chrono>

class BookPrinter : public hyperliquid::WSMessageHandler, public hyperliquid::WebsocketListener {
public:
    // hyperliquid::WebsocketListener
    void onMessage(const std::string& message) override {
        messageParser.crack(message, *this);
    }

    void onConnected() override {
        std::cout << "Connected" << std::endl;
    }

    void onDisconnected(bool hasError, const std::string& errMsg) override {
        std::cout << "Disconnected" << std::endl;
    }

    // hyperliquid::WSMessageHandler
    void onL2BookLevel(const hyperliquid::L2BookUpdate& book, const hyperliquid::PriceLevel& level) override {
        std::cout << book.time << " [L2Book] " << book.coin
                  << (level.side == hyperliquid::Side::Bid ? " BID " : " ASK ")
                  << level.sz << " @ " << level.px
                  << " (" << level.n << ")" << std::endl;
    }

    void onBbo(const hyperliquid::BboUpdate& bbo) override {
        std::cout << bbo.time << " [BBO] " << bbo.coin;
        if (bbo.hasBid)
            std::cout << " BID " << bbo.bid.sz << " @ " << bbo.bid.px;
        if (bbo.hasAsk)
            std::cout << " ASK " << bbo.ask.sz << " @ " << bbo.ask.px;
        std::cout << std::endl;
    }

    void onTrade(const hyperliquid::Trade& trade) override {
        std::cout << trade.time << " [TRADE] " << trade.coin
                  << " " << trade.side
                  << " " << trade.sz << " @ " << trade.px << std::endl;
    }

private:
    hyperliquid::WSMessageParser messageParser;
};

int main() {
    BookPrinter printer;
    hyperliquid::MarketData md(hyperliquid::Environment::Mainnet, printer);

    std::cout << "Subscribing to BTC l2Book + bbo + trades for 5 seconds..." << std::endl;
    md.start();

    md.subscribe(hyperliquid::SubscriptionType::L2Book, {{"coin", "BTC"}});
    md.subscribe(hyperliquid::SubscriptionType::Bbo, {{"coin", "BTC"}});
    md.subscribe(hyperliquid::SubscriptionType::Trades, {{"coin", "BTC"}});

    std::this_thread::sleep_for(std::chrono::seconds(5));
    md.stop();

    return 0;
}
