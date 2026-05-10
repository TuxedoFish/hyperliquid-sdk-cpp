#include <thread>
#include <chrono>

#include <spdlog/spdlog.h>

#include "test_config.h"
#include "hyperliquid/rest/RestApi.h"
#include "hyperliquid/rest/RestApiMessageParser.h"
#include "hyperliquid/rest/RestEndpointListener.h"
#include "hyperliquid/websocket/WebsocketApi.h"
#include "hyperliquid/websocket/WebsocketApiListener.h"
#include "hyperliquid/websocket/WebsocketMessageHandler.h"
#include "hyperliquid/websocket/WebsocketMessageParser.h"

class SubaccountListener : public hyperliquid::WebsocketMessageHandler,
                           public hyperliquid::WebsocketApiListener,
                           public hyperliquid::RestEndpointListener {
public:
    SubaccountListener(const std::string& subaccountAddress)
        : restParser(*this), ws_(nullptr), cloid_(hyperliquid::generateCloid()),
          subaccountAddress_(subaccountAddress) {}

    void setWebsocket(hyperliquid::WebsocketApi& ws) { ws_ = &ws; }

    void onMessage(const std::string& message) override {
        messageParser.crack(message, *this);
    }

    void onPostResponse(const std::string& message, hyperliquid::RestEndpointType type,
                         std::optional<uint64_t> correlationId) override {
        restParser.parse(message, type, correlationId);
    }

    void onConnected() override {
        spdlog::info("Connected, subscribing for subaccount {} ...", subaccountAddress_);
        ws_->subscribe(hyperliquid::SubscriptionType::OrderUpdates,
                       {{"user", subaccountAddress_}});

        hyperliquid::OrderRequest order;
        order.asset = "ETH";
        order.isBuy = true;
        order.price = 1000.0;
        order.size = 0.01;
        order.reduceOnly = false;
        order.limit = hyperliquid::LimitOrderType{hyperliquid::Tif::Gtc};
        order.cloid = cloid_;

        spdlog::info("Placing order on subaccount via vaultAddress...");
        ws_->placeOrder({order}, hyperliquid::Grouping::Na,
                        std::nullopt, 1, subaccountAddress_);
    }

    void onDisconnected(bool hasError, const std::string& errMsg) override {
        spdlog::info("Disconnected");
    }

    void onPlaceOrder(const hyperliquid::PlaceOrderResponse& response,
                       std::optional<uint64_t> correlationId) override {
        if (correlationId)
            spdlog::info("Ack received for correlationId -> {}", *correlationId);
        spdlog::info("Place order: status={}", response.status);
        if (response.status != "ok" || response.statuses.empty()) return;

        auto& first = response.statuses[0];
        if (first.resting) {
            spdlog::info("Order resting oid={}, cancelling by cloid...", first.resting->oid);
            hyperliquid::CancelByCloidRequest cancel;
            cancel.cloid = cloid_;
            ws_->cancelOrderByCloid({cancel}, 2, subaccountAddress_);
        } else if (first.error) {
            spdlog::error("Order error: {}", *first.error);
        }
    }

    void onCancelOrder(const hyperliquid::CancelOrderResponse& response,
                        std::optional<uint64_t> correlationId) override {
        if (correlationId)
            spdlog::info("Ack received for correlationId -> {}", *correlationId);
        spdlog::info("Cancel order: status={}", response.status);
    }

    void onOrderUpdate(const hyperliquid::OrderUpdate& update) override {
        spdlog::info("Order update: coin={} side={} status={} oid={} sz={} limitPx={}",
                     update.coin, update.side, hyperliquid::toString(update.status),
                     update.oid, update.sz, update.limitPx);
    }

private:
    hyperliquid::WebsocketMessageParser messageParser;
    hyperliquid::RestApiMessageParser restParser;
    hyperliquid::WebsocketApi* ws_;
    std::string cloid_;
    std::string subaccountAddress_;
};

int main() {
    auto tc = loadTestConfig();
    hyperliquid::setLogLevel(hyperliquid::LogLevel::Debug);

    if (!tc.subaccount) {
        spdlog::error("No subaccount address in test.json - set \"subaccount\" field");
        return 1;
    }

    hyperliquid::ApiConfig config;
    config.env = hyperliquid::Environment::Testnet;
    config.wallet = tc.wallet;

    SubaccountListener listener(*tc.subaccount);
    hyperliquid::WebsocketApi websocket(config, listener);
    listener.setWebsocket(websocket);

    spdlog::info("Starting websocket for subaccount test...");
    spdlog::info("Master wallet: {}", tc.wallet.accountAddress);
    spdlog::info("Subaccount (vaultAddress): {}", *tc.subaccount);
    websocket.start();

    std::this_thread::sleep_for(std::chrono::seconds(15));
    websocket.stop();

    return 0;
}
