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

class OrderListener : public hyperliquid::WebsocketMessageHandler,
                      public hyperliquid::WebsocketApiListener,
                      public hyperliquid::RestEndpointListener {
public:
    OrderListener() : restParser(*this), ws_(nullptr), cloid_(hyperliquid::generateCloid()) {}
    void setWebsocket(hyperliquid::WebsocketApi& ws) { ws_ = &ws; }

    void onMessage(const std::string& message) override {
        messageParser.crack(message, *this);
    }

    void onPostResponse(const std::string& message, hyperliquid::RestEndpointType type,
                         std::optional<uint64_t> correlationId) override {
        restParser.parse(message, type, correlationId);
    }

    void onPerpAssetCtx(const hyperliquid::PerpAssetCtx& ctx) override {
        if (ctx.coin != "ETH" || orderPlaced_ || !ctx.hasMidPx) return;
        orderPlaced_ = true;

        midPx_ = ctx.midPx;
        spdlog::info("ETH mid={}, placing order 5% below...", midPx_);

        double orderPx = std::floor(midPx_ * 0.95 * 10.0) / 10.0;

        hyperliquid::OrderRequest order;
        order.asset = "ETH";
        order.isBuy = true;
        order.price = orderPx;
        order.size = 0.01;
        order.reduceOnly = false;
        order.limit = hyperliquid::LimitOrderType{hyperliquid::Tif::Alo};
        order.cloid = cloid_;

        ws_->placeOrder({order}, hyperliquid::Grouping::Na);
    }

    void onPlaceOrder(const hyperliquid::PlaceOrderResponse& response,
                       std::optional<uint64_t>) override {
        spdlog::info("Place order: status={}", response.status);
        if (response.status != "ok" || response.statuses.empty()) return;

        auto& first = response.statuses[0];
        if (first.resting)
        {
            double modifyPx = std::floor(midPx_ * 0.94 * 10.0) / 10.0;
            spdlog::info("Order resting oid={}, modifying to {}...", first.resting->oid, modifyPx);

            hyperliquid::OrderRequest modified;
            modified.asset = "ETH";
            modified.isBuy = true;
            modified.price = modifyPx;
            modified.size = 0.01;
            modified.reduceOnly = false;
            modified.limit = hyperliquid::LimitOrderType{hyperliquid::Tif::Alo};
            modified.cloid = cloid_;

            hyperliquid::ModifyRequest modify;
            modify.oid = first.resting->oid;
            modify.order = modified;
            ws_->modifyOrder(modify);
        }
    }

    void onModifyOrder(const hyperliquid::ModifyOrderResponse& response,
                        std::optional<uint64_t>) override {
        spdlog::info("Modify order: status={}", response.status);
        if (response.status != "ok") return;

        spdlog::info("Modified, cancelling by cloid...");
        hyperliquid::CancelByCloidRequest cancel;
        cancel.asset = "ETH";
        cancel.cloid = cloid_;
        ws_->cancelOrderByCloid({cancel});
    }

    void onCancelOrder(const hyperliquid::CancelOrderResponse& response,
                        std::optional<uint64_t>) override {
        spdlog::info("Cancel order: status={}", response.status);
        spdlog::info("Unsubscribing...");
        ws_->unsubscribe(hyperliquid::SubscriptionType::OrderUpdates);
        ws_->unsubscribe(hyperliquid::SubscriptionType::UserFills);
        ws_->unsubscribe(hyperliquid::SubscriptionType::ActiveAssetCtx, {{"coin", "ETH"}});
    }

    void onOrderUpdate(const hyperliquid::OrderUpdate& update) override {
        spdlog::info("Order update: coin={} side={} status={} oid={} sz={} limitPx={}",
                      update.coin, update.side, hyperliquid::toString(update.status), update.oid, update.sz, update.limitPx);
    }

    void onUserFill(const hyperliquid::Fill& fill) override {
        spdlog::info("Fill: coin={} side={} px={} sz={} oid={} fee={} snapshot={}",
                      fill.coin, fill.side, fill.px, fill.sz, fill.oid, fill.fee, fill.isSnapshot);
    }

    void onConnected() override {
        spdlog::info("Connected, subscribing...");
        ws_->subscribe(hyperliquid::SubscriptionType::OrderUpdates);
        ws_->subscribe(hyperliquid::SubscriptionType::UserFills);
        ws_->subscribe(hyperliquid::SubscriptionType::ActiveAssetCtx, {{"coin", "ETH"}});
    }

    void onDisconnected(bool hasError, const std::string& errMsg) override {
        spdlog::info("Disconnected: hasError={} errMsg={}", hasError, errMsg);
    }

private:
    hyperliquid::WebsocketMessageParser messageParser;
    hyperliquid::RestApiMessageParser restParser;
    hyperliquid::WebsocketApi* ws_;
    std::string cloid_;
    double midPx_ = 0.0;
    bool orderPlaced_ = false;
};

int main() {
    auto wallet = loadWalletFromConfig();
    hyperliquid::setLogLevel(hyperliquid::LogLevel::Debug);

    hyperliquid::ApiConfig config;
    config.env = hyperliquid::Environment::Testnet;
    config.wallet = wallet;

    OrderListener listener;
    hyperliquid::WebsocketApi websocket(config, listener);
    listener.setWebsocket(websocket);

    spdlog::info("Starting websocket...");
    websocket.start();

    std::this_thread::sleep_for(std::chrono::seconds(15));
    websocket.stop();

    return 0;
}
