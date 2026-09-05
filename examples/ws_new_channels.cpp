#include <thread>
#include <chrono>

#include <spdlog/spdlog.h>

#include "test_config.h"
#include "hyperliquid/websocket/WebsocketApi.h"
#include "hyperliquid/websocket/WebsocketApiListener.h"
#include "hyperliquid/websocket/WebsocketMessageHandler.h"
#include "hyperliquid/websocket/WebsocketMessageParser.h"

class NewChannelsListener : public hyperliquid::WebsocketMessageHandler,
                             public hyperliquid::WebsocketApiListener {
public:
    void setWebsocket(hyperliquid::WebsocketApi& ws) { ws_ = &ws; }

    void onMessage(const std::string& message) override {
        messageParser.crack(message, *this);
    }

    void onTwapStates(const hyperliquid::TwapStatesUpdate& update) override {
        spdlog::info("twapStates: dex={} user={} count={}", update.dex, update.user, update.states.size());
    }

    void onNotification(const hyperliquid::Notification& n) override {
        spdlog::info("notification: {}", n.notification);
    }

    void onUserTwapSliceFill(const hyperliquid::TwapSliceFill& fill) override {
        spdlog::info("userTwapSliceFills: twapId={} coin={} px={} sz={}",
                      fill.twapId, fill.fill.coin, fill.fill.px, fill.fill.sz);
    }

    void onUserTwapHistory(const hyperliquid::TwapHistoryEntry& entry) override {
        spdlog::info("userTwapHistory: status={} snapshot={}",
                      hyperliquid::toString(entry.status), entry.isSnapshot);
    }

    void onActiveAssetData(const hyperliquid::ActiveAssetData& data) override {
        spdlog::info("activeAssetData: coin={} user={} maxTradeSzLong={}", data.coin, data.user, data.maxTradeSzLong);
    }

    void onSpotState(const hyperliquid::SpotStateUpdate& update) override {
        spdlog::info("spotState: user={} balances={}", update.user, update.balances.size());
        for (const auto& b : update.balances) {
            spdlog::info("  balance: coin={} total={}", b.coin, b.total);
        }
    }

    void onAllDexsClearinghouseState(const hyperliquid::AllDexsClearinghouseStateUpdate& update) override {
        spdlog::info("allDexsClearinghouseState: user={} dexs={}", update.user, update.states.size());
    }

    void onAllDexsAssetCtxs(const hyperliquid::AllDexsAssetCtxsUpdate& update) override {
        spdlog::info("allDexsAssetCtxs: dexs={}", update.dexs.size());
        for (const auto& d : update.dexs) {
            spdlog::info("  dex={} ctxs={}", d.dex, d.ctxs.size());
        }
    }

    void onFastAssetCtx(const hyperliquid::FastAssetCtx& ctx) override {
        if (!fastAssetCtxSeen_) {
            fastAssetCtxSeen_ = true;
            spdlog::info("fastAssetCtxs: coin={} hasMarkPx={} markPx={}", ctx.coin, ctx.hasMarkPx, ctx.markPx);
        }
    }

    void onConnected() override {
        spdlog::info("Connected, subscribing to new channels...");
        ws_->subscribe(hyperliquid::SubscriptionType::TwapStates);
        ws_->subscribe(hyperliquid::SubscriptionType::Notification);
        ws_->subscribe(hyperliquid::SubscriptionType::UserTwapSliceFills);
        ws_->subscribe(hyperliquid::SubscriptionType::UserTwapHistory);
        ws_->subscribe(hyperliquid::SubscriptionType::ActiveAssetData, {{"coin", "BTC"}});
        ws_->subscribe(hyperliquid::SubscriptionType::SpotState);
        ws_->subscribe(hyperliquid::SubscriptionType::AllDexsClearinghouseState);
        ws_->subscribe(hyperliquid::SubscriptionType::AllDexsAssetCtxs);
        ws_->subscribe(hyperliquid::SubscriptionType::FastAssetCtxs);
    }

    void onDisconnected(bool hasError, const std::string& errMsg) override {
        spdlog::info("Disconnected: hasError={} errMsg={}", hasError, errMsg);
    }

private:
    hyperliquid::WebsocketMessageParser messageParser;
    hyperliquid::WebsocketApi* ws_ = nullptr;
    bool fastAssetCtxSeen_ = false; // fastAssetCtxs streams continuously; log only the first
};

int main() {
    auto wallet = loadWalletFromConfig();
    hyperliquid::setLogLevel(hyperliquid::LogLevel::Info);

    hyperliquid::ApiConfig config;
    config.env = hyperliquid::Environment::Testnet;
    config.wallet = wallet;

    NewChannelsListener listener;
    hyperliquid::WebsocketApi websocket(config, listener);
    listener.setWebsocket(websocket);

    spdlog::info("Starting websocket...");
    websocket.start();

    std::this_thread::sleep_for(std::chrono::seconds(15));
    websocket.stop();

    return 0;
}
