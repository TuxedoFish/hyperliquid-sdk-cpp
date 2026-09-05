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

class OutcomeOrderListener : public hyperliquid::WebsocketMessageHandler,
                             public hyperliquid::WebsocketApiListener,
                             public hyperliquid::RestEndpointListener {
public:
    OutcomeOrderListener(int outcomeIndex, int outcomeSide)
        : restParser(*this), ws_(nullptr), cloid_(hyperliquid::generateCloid()),
          outcomeIndex_(outcomeIndex), outcomeSide_(outcomeSide) {}

    void setWebsocket(hyperliquid::WebsocketApi& ws) { ws_ = &ws; }

    void onMessage(const std::string& message) override {
        messageParser.crack(message, *this);
    }

    void onPostResponse(const std::string& message, hyperliquid::RestEndpointType type,
                         std::optional<uint64_t> correlationId) override {
        restParser.parse(message, type, correlationId);
    }

    void onConnected() override {
        spdlog::info("Connected, subscribing and placing outcome order...");
        ws_->subscribe(hyperliquid::SubscriptionType::OrderUpdates);

        hyperliquid::OrderRequest order;
        order.assetId = hyperliquid::outcomeAssetId(outcomeIndex_, outcomeSide_);
        order.isBuy = true;
        order.price = 0.10;
        order.size = 100.0; // Min size is 10 USDH
        order.reduceOnly = false;
        order.limit = hyperliquid::LimitOrderType{hyperliquid::Tif::Gtc};
        order.cloid = cloid_;

        spdlog::info("Placing outcome order: outcomeIndex={} side={} (assetId={})",
                     outcomeIndex_, outcomeSide_,
                     hyperliquid::outcomeAssetId(outcomeIndex_, outcomeSide_));
        ws_->placeOrder({order}, hyperliquid::Grouping::Na);
    }

    void onDisconnected(bool hasError, const std::string& errMsg) override {
        spdlog::info("Disconnected: hasError={} errMsg={}", hasError, errMsg);
    }

    void onPlaceOrder(const hyperliquid::PlaceOrderResponse& response,
                       std::optional<uint64_t>) override {
        spdlog::info("Place order: status={}", response.status);
        if (response.status != "ok" || response.statuses.empty()) return;

        auto& first = response.statuses[0];
        if (first.resting) {
            spdlog::info("Order resting oid={}, modifying to px=0.20 sz=200...", first.resting->oid);

            hyperliquid::OrderRequest modified;
            modified.assetId = hyperliquid::outcomeAssetId(outcomeIndex_, outcomeSide_);
            modified.isBuy = true;
            modified.price = 0.20;
            modified.size = 200.0;
            modified.reduceOnly = false;
            modified.limit = hyperliquid::LimitOrderType{hyperliquid::Tif::Gtc};
            modified.cloid = cloid_;

            hyperliquid::ModifyRequest modify;
            modify.oid = first.resting->oid;
            modify.order = modified;
            ws_->modifyOrder(modify);
        } else if (first.error) {
            spdlog::error("Order error: {}", *first.error);
        }
    }

    void onModifyOrder(const hyperliquid::ModifyOrderResponse& response,
                        std::optional<uint64_t>) override {
        spdlog::info("Modify order: status={}", response.status);
        if (response.status != "ok") return;

        spdlog::info("Modified, cancelling by cloid...");
        hyperliquid::CancelByCloidRequest cancel;
        cancel.assetId = hyperliquid::outcomeAssetId(outcomeIndex_, outcomeSide_);
        cancel.cloid = cloid_;
        ws_->cancelOrderByCloid({cancel});
    }

    void onCancelOrder(const hyperliquid::CancelOrderResponse& response,
                        std::optional<uint64_t>) override {
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

    // Trade the "Yes" side (side=0)
    int outcomeSide = 0;
    spdlog::info("Trading Yes side: assetId={}", hyperliquid::outcomeAssetId(outcomeIndex, outcomeSide));

    OutcomeOrderListener listener(outcomeIndex, outcomeSide);
    hyperliquid::WebsocketApi websocket(config, listener);
    listener.setWebsocket(websocket);

    spdlog::info("Starting websocket for outcome order test...");
    websocket.start();

    std::this_thread::sleep_for(std::chrono::seconds(15));
    websocket.stop();

    return 0;
}
