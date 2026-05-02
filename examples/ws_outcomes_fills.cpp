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

class OutcomeFillsListener : public hyperliquid::WebsocketMessageHandler,
                             public hyperliquid::WebsocketApiListener,
                             public hyperliquid::RestEndpointListener {
public:
    OutcomeFillsListener(int outcomeIndex, int outcomeSide)
        : restParser(*this), ws_(nullptr), cloid_(hyperliquid::generateCloid()),
          outcomeIndex_(outcomeIndex), outcomeSide_(outcomeSide) {}

    void setWebsocket(hyperliquid::WebsocketApi& ws) { ws_ = &ws; }

    void onMessage(const std::string& message) override {
        messageParser.crack(message, *this);
    }

    void onPostResponse(const std::string& message, hyperliquid::RestEndpointType type) override {
        restParser.parse(message, type);
    }

    void onConnected() override {
        spdlog::info("Connected, subscribing and placing outcome sell order...");
        ws_->subscribe(hyperliquid::SubscriptionType::OrderUpdates);
        ws_->subscribe(hyperliquid::SubscriptionType::UserFills);

        // Sell (offer) into the empty offer side - IOC to cross the spread
        hyperliquid::OrderRequest order;
        order.isOutcome = true;
        order.outcomeIndex = outcomeIndex_;
        order.outcomeSide = outcomeSide_;
        order.isBuy = false;
        order.price = 0.50;
        order.size = 100.0;
        order.reduceOnly = false;
        order.limit = hyperliquid::LimitOrderType{hyperliquid::Tif::Ioc};
        order.cloid = cloid_;

        spdlog::info("Placing outcome SELL order: outcomeIndex={} side={} px=0.50 sz=100 (assetId={})",
                     outcomeIndex_, outcomeSide_,
                     hyperliquid::outcomeAssetId(outcomeIndex_, outcomeSide_));
        ws_->placeOrder({order}, hyperliquid::Grouping::Na);
    }

    void onDisconnected(bool hasError, const std::string& errMsg) override {
        spdlog::info("Disconnected");
    }

    void onPlaceOrder(const hyperliquid::PlaceOrderResponse& response) override {
        spdlog::info("Place order: status={}", response.status);
        if (response.status != "ok" || response.statuses.empty()) return;

        auto& first = response.statuses[0];
        if (first.filled) {
            spdlog::info("Order filled: totalSz={} avgPx={} oid={}",
                         first.filled->totalSz, first.filled->avgPx, first.filled->oid);
        } else if (first.resting) {
            spdlog::info("Order resting oid={}", first.resting->oid);
        } else if (first.error) {
            spdlog::error("Order error: {}", *first.error);
        }
    }

    void onOrderUpdate(const hyperliquid::OrderUpdate& update) override {
        spdlog::info("Order update: coin={} side={} status={} oid={} sz={} limitPx={}",
                     update.coin, update.side, hyperliquid::toString(update.status),
                     update.oid, update.sz, update.limitPx);
    }

    void onUserFill(const hyperliquid::Fill& fill) override {
        spdlog::info("Fill: coin={} side={} px={} sz={} oid={} fee={} dir={} closedPnl={} snapshot={}",
                     fill.coin, fill.side, fill.px, fill.sz, fill.oid, fill.fee,
                     fill.dir, fill.closedPnl, fill.isSnapshot);
    }

private:
    hyperliquid::WebsocketMessageParser messageParser;
    hyperliquid::RestApiMessageParser restParser;
    hyperliquid::WebsocketApi* ws_;
    std::string cloid_;
    int outcomeIndex_;
    int outcomeSide_;
};

int main() {
    auto wallet = loadWalletFromConfig();
    hyperliquid::setLogLevel(hyperliquid::LogLevel::Debug);

    hyperliquid::ApiConfig config;
    config.env = hyperliquid::Environment::Testnet;
    config.wallet = wallet;

    // Fetch outcomeMeta and find BTC 1d market
    hyperliquid::RestApi restApi(config);
    hyperliquid::RestApiMessageParser parser;

    auto outcomeMeta = parser.parseOutcomeMeta(restApi.outcomeMeta());
    spdlog::info("Fetched {} outcomes, searching for underlying=BTC period=1d...", outcomeMeta.outcomes.size());

    int outcomeIndex = -1;
    for (const auto& outcome : outcomeMeta.outcomes) {
        if (outcome.description.underlying == "BTC" && outcome.description.period == "1d") {
            outcomeIndex = outcome.outcome;
            spdlog::info("Found BTC 1d outcome: index={} class={} expiry={} targetPrice={}",
                         outcome.outcome, outcome.description.outcomeClass,
                         outcome.description.expiry, outcome.description.targetPrice);
            break;
        }
    }

    if (outcomeIndex < 0) {
        spdlog::error("No BTC 1d outcome found");
        return 1;
    }

    // Trade the "Yes" side (side=0)
    int outcomeSide = 0;
    spdlog::info("Trading Yes side (SELL): assetId={}", hyperliquid::outcomeAssetId(outcomeIndex, outcomeSide));

    OutcomeFillsListener listener(outcomeIndex, outcomeSide);
    hyperliquid::WebsocketApi websocket(config, listener);
    listener.setWebsocket(websocket);

    spdlog::info("Starting websocket for outcome fills test...");
    websocket.start();

    std::this_thread::sleep_for(std::chrono::seconds(15));
    websocket.stop();

    return 0;
}
