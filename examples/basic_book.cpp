#include <hyperliquid/types/ResponseTypes.h>
#include <iostream>
#include <thread>
#include <chrono>

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
    hyperliquid::WebsocketMessageParser messageParser;
};

int main() {
    BookPrinter printer;
    hyperliquid::WebsocketApi websocket(hyperliquid::Environment::Mainnet, printer);

    std::cout << "Subscribing to BTC l2Book + bbo + trades for 5 seconds..." << std::endl;
    websocket.start();

    websocket.subscribe(hyperliquid::SubscriptionType::L2Book, {{"coin", "BTC"}});
    websocket.subscribe(hyperliquid::SubscriptionType::Bbo, {{"coin", "BTC"}});
    websocket.subscribe(hyperliquid::SubscriptionType::Trades, {{"coin", "BTC"}});

    std::this_thread::sleep_for(std::chrono::seconds(5));
    websocket.stop();

    return 0;
}
