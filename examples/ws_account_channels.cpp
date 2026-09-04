#include <thread>
#include <chrono>

#include <spdlog/spdlog.h>

#include "test_config.h"
#include "hyperliquid/websocket/WebsocketApi.h"
#include "hyperliquid/websocket/WebsocketApiListener.h"
#include "hyperliquid/websocket/WebsocketMessageHandler.h"
#include "hyperliquid/websocket/WebsocketMessageParser.h"

class AccountChannelsListener : public hyperliquid::WebsocketMessageHandler,
                                 public hyperliquid::WebsocketApiListener {
public:
    void setWebsocket(hyperliquid::WebsocketApi& ws) { ws_ = &ws; }

    void onMessage(const std::string& message) override {
        messageParser.crack(message, *this);
    }

    void onUserFundingUpdate(const hyperliquid::UserFunding& funding) override {
        spdlog::info("userFundings: coin={} usdc={} szi={} fundingRate={} snapshot={}",
                      funding.coin, funding.usdc, funding.szi, funding.fundingRate, funding.isSnapshot);
    }

    void onLedgerUpdate(const hyperliquid::LedgerUpdate& update) override {
        spdlog::info("userNonFundingLedgerUpdates: type={} hash={} usdc={} snapshot={}",
                      hyperliquid::toString(update.type), update.hash, update.usdc, update.isSnapshot);
    }

    void onWebData3(const hyperliquid::WebData3Update& update) override {
        spdlog::info("webData3: user={} serverTime={} cumLedger={} perpDexStates={}",
                      update.userState.user, update.userState.serverTime,
                      update.userState.cumLedger, update.perpDexStates.size());
    }

    void onClearinghouseState(const hyperliquid::ClearinghouseStateUpdate& update) override {
        spdlog::info("clearinghouseState: dex={} user={} positions={} accountValue={} withdrawable={}",
                      update.dex, update.user, update.state.assetPositions.size(),
                      update.state.marginSummary.accountValue, update.state.withdrawable);
    }

    void onOpenOrdersSnapshot(const hyperliquid::OpenOrdersUpdate& update) override {
        spdlog::info("openOrders: dex={} user={} count={}", update.dex, update.user, update.orders.size());
        for (const auto& order : update.orders) {
            spdlog::info("  order: coin={} side={} limitPx={} sz={} oid={}",
                          order.coin, order.side, order.limitPx, order.sz, order.oid);
        }
    }

    void onConnected() override {
        spdlog::info("Connected, subscribing to account-read channels...");
        ws_->subscribe(hyperliquid::SubscriptionType::UserFundings);
        ws_->subscribe(hyperliquid::SubscriptionType::UserNonFundingLedgerUpdates);
        ws_->subscribe(hyperliquid::SubscriptionType::WebData3);
        ws_->subscribe(hyperliquid::SubscriptionType::ClearingHouseState);
        ws_->subscribe(hyperliquid::SubscriptionType::OpenOrders);
    }

    void onDisconnected(bool hasError, const std::string& errMsg) override {
        spdlog::info("Disconnected");
    }

private:
    hyperliquid::WebsocketMessageParser messageParser;
    hyperliquid::WebsocketApi* ws_ = nullptr;
};

int main() {
    auto wallet = loadWalletFromConfig();
    hyperliquid::setLogLevel(hyperliquid::LogLevel::Debug);

    hyperliquid::ApiConfig config;
    config.env = hyperliquid::Environment::Mainnet;
    config.wallet = wallet;

    AccountChannelsListener listener;
    hyperliquid::WebsocketApi websocket(config, listener);
    listener.setWebsocket(websocket);

    spdlog::info("Starting websocket...");
    websocket.start();

    std::this_thread::sleep_for(std::chrono::seconds(15));
    websocket.stop();

    return 0;
}
