#include <thread>
#include <chrono>

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

    void onPostResponse(const std::string& message, hyperliquid::RestEndpointType type) override {
        restParser.parse(message, type);
    }

    void onPlaceOrder(const hyperliquid::PlaceOrderResponse& response) override {
        spdlog::info("Place order: status={}", response.status);
        if (response.status != "ok" || response.statuses.empty()) return;

        auto& first = response.statuses[0];
        if (first.resting)
        {
            spdlog::info("Order resting oid={}, modifying...", first.resting->oid);
            hyperliquid::OrderRequest modified;
            modified.asset = "ETH";
            modified.isBuy = true;
            modified.price = 1801.0;
            modified.size = 0.01;
            modified.reduceOnly = false;
            modified.limit = hyperliquid::LimitOrderType{hyperliquid::Tif::Gtc};
            modified.cloid = cloid_;

            hyperliquid::ModifyRequest modify;
            modify.oid = first.resting->oid;
            modify.order = modified;
            ws_->modifyOrder(modify);
        }
    }

    void onModifyOrder(const hyperliquid::ModifyOrderResponse& response) override {
        spdlog::info("Modify order: status={}", response.status);
        if (response.status != "ok") return;

        spdlog::info("Modified, cancelling by cloid...");
        hyperliquid::CancelByCloidRequest cancel;
        cancel.asset = "ETH";
        cancel.cloid = cloid_;
        ws_->cancelOrderByCloid({cancel});
    }

    void onCancelOrder(const hyperliquid::CancelOrderResponse& response) override {
        spdlog::info("Cancel order: status={}", response.status);
    }

    void onConnected() override {
        spdlog::info("Connected, placing order...");

        hyperliquid::OrderRequest order;
        order.asset = "ETH";
        order.isBuy = true;
        order.price = 1800.0;
        order.size = 0.01;
        order.reduceOnly = false;
        order.limit = hyperliquid::LimitOrderType{hyperliquid::Tif::Gtc};
        order.cloid = cloid_;

        ws_->placeOrder({order}, hyperliquid::Grouping::Na);
    }

    void onDisconnected(bool hasError, const std::string& errMsg) override {
        spdlog::info("Disconnected");
    }

private:
    hyperliquid::WebsocketMessageParser messageParser;
    hyperliquid::RestApiMessageParser restParser;
    hyperliquid::WebsocketApi* ws_;
    std::string cloid_;
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

    std::this_thread::sleep_for(std::chrono::seconds(10));
    websocket.stop();

    return 0;
}
