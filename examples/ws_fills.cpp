#include <thread>
#include <chrono>
#include <cmath>

#include <spdlog/spdlog.h>

#include "test_config.h"
#include "hyperliquid/websocket/WebsocketApi.h"
#include "hyperliquid/websocket/WebsocketApiListener.h"
#include "hyperliquid/websocket/WebsocketMessageHandler.h"
#include "hyperliquid/websocket/WebsocketMessageParser.h"
#include "hyperliquid/rest/RestApiMessageParser.h"
#include "hyperliquid/rest/RestEndpointListener.h"

class FillListener : public hyperliquid::WebsocketMessageHandler,
                     public hyperliquid::WebsocketApiListener,
                     public hyperliquid::RestEndpointListener {
public:
    FillListener() : restParser(*this), ws_(nullptr) {}
    void setWebsocket(hyperliquid::WebsocketApi& ws) { ws_ = &ws; }

    void onMessage(const std::string& message) override {
        messageParser.crack(message, *this);
    }

    void onPostResponse(const std::string& message, hyperliquid::RestEndpointType type) override {
        restParser.parse(message, type);
    }

    void onPerpAssetCtx(const hyperliquid::PerpAssetCtx& ctx) override {
        if (ctx.coin != "ETH" || orderPlaced_ || !ctx.hasMidPx) return;
        orderPlaced_ = true;

        // IOC sell at bid- to cross the spread immediately
        double crossPx = std::floor(ctx.midPx * 0.99 * 10.0) / 10.0;
        spdlog::info("ETH mid={}, sending IOC sell at {} to cross...", ctx.midPx, crossPx);

        hyperliquid::OrderRequest order;
        order.asset = "ETH";
        order.isBuy = false;
        order.price = crossPx;
        order.size = 0.01;
        order.reduceOnly = false;
        order.limit = hyperliquid::LimitOrderType{hyperliquid::Tif::Ioc};

        ws_->placeOrder({order}, hyperliquid::Grouping::Na);
    }

    void onPlaceOrder(const hyperliquid::PlaceOrderResponse& response) override {
        spdlog::info("Place order: status={}", response.status);
        for (const auto& s : response.statuses)
        {
            if (s.filled)
                spdlog::info("  Filled oid={} avgPx={} totalSz={}", s.filled->oid, s.filled->avgPx, s.filled->totalSz);
            else if (s.resting)
                spdlog::info("  Resting oid={}", s.resting->oid);
            else if (s.error)
                spdlog::info("  Error: {}", *s.error);
        }
    }

    void onOrderUpdate(const hyperliquid::OrderUpdate& update) override {
        spdlog::info("Order update: coin={} side={} status={} oid={} sz={} limitPx={}",
                      update.coin, update.side, hyperliquid::toString(update.status), update.oid, update.sz, update.limitPx);
    }

    void onUserFill(const hyperliquid::Fill& fill) override {
        spdlog::info("Fill: coin={} side={} px={} sz={} dir={} closedPnl={} fee={} oid={}",
                      fill.coin, fill.side, fill.px, fill.sz, fill.dir, fill.closedPnl, fill.fee, fill.oid);

        // Close the position with an IOC buy
        if (!closeSent_)
        {
            closeSent_ = true;
            double closePx = std::ceil(fill.px * 1.01 * 10.0) / 10.0;
            spdlog::info("Closing position with IOC buy at {}...", closePx);

            hyperliquid::OrderRequest close;
            close.asset = "ETH";
            close.isBuy = true;
            close.price = closePx;
            close.size = fill.sz;
            close.reduceOnly = true;
            close.limit = hyperliquid::LimitOrderType{hyperliquid::Tif::Ioc};

            ws_->placeOrder({close}, hyperliquid::Grouping::Na);
        }
    }

    void onConnected() override {
        spdlog::info("Connected, subscribing...");
        ws_->subscribe(hyperliquid::SubscriptionType::OrderUpdates);
        ws_->subscribe(hyperliquid::SubscriptionType::UserFills);
        ws_->subscribe(hyperliquid::SubscriptionType::ActiveAssetCtx, {{"coin", "ETH"}});
    }

    void onDisconnected(bool hasError, const std::string& errMsg) override {
        spdlog::info("Disconnected");
    }

private:
    hyperliquid::WebsocketMessageParser messageParser;
    hyperliquid::RestApiMessageParser restParser;
    hyperliquid::WebsocketApi* ws_;
    bool orderPlaced_ = false;
    bool closeSent_ = false;
};

int main() {
    auto wallet = loadWalletFromConfig();
    hyperliquid::setLogLevel(hyperliquid::LogLevel::Debug);

    hyperliquid::ApiConfig config;
    config.env = hyperliquid::Environment::Testnet;
    config.wallet = wallet;

    FillListener listener;
    hyperliquid::WebsocketApi websocket(config, listener);
    listener.setWebsocket(websocket);

    spdlog::info("Starting websocket...");
    websocket.start();

    std::this_thread::sleep_for(std::chrono::seconds(15));
    websocket.stop();

    return 0;
}
