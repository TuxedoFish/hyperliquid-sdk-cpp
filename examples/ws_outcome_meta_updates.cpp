#include <thread>
#include <chrono>

#include <spdlog/spdlog.h>

#include "test_config.h"
#include "hyperliquid/websocket/WebsocketApi.h"
#include "hyperliquid/websocket/WebsocketApiListener.h"
#include "hyperliquid/websocket/WebsocketMessageHandler.h"
#include "hyperliquid/websocket/WebsocketMessageParser.h"

class OutcomeMetaUpdatesListener : public hyperliquid::WebsocketMessageHandler,
                                    public hyperliquid::WebsocketApiListener {
public:
    void setWebsocket(hyperliquid::WebsocketApi& ws) { ws_ = &ws; }

    void onMessage(const std::string& message) override {
        messageParser.crack(message, *this);
    }

    void onOutcomeMetaUpdate(const hyperliquid::OutcomeMetaUpdate& update) override {
        using hyperliquid::OutcomeMetaUpdateType;
        switch (update.type) {
        case OutcomeMetaUpdateType::OutcomeCreated:
            spdlog::info("outcomeCreated: outcome={} name={} quoteToken={}",
                         update.outcome.outcome, update.outcome.name, update.outcome.quoteToken);
            break;
        case OutcomeMetaUpdateType::OutcomeSettled:
            spdlog::info("outcomeSettled: outcome={}", update.settledOutcome);
            break;
        case OutcomeMetaUpdateType::QuestionUpdated:
            spdlog::info("questionUpdated: question={} name={}", update.question.question, update.question.name);
            break;
        case OutcomeMetaUpdateType::QuestionSettled:
            spdlog::info("questionSettled: question={}", update.settledQuestion);
            break;
        case OutcomeMetaUpdateType::Unknown:
            spdlog::warn("outcomeMetaUpdates: unrecognized variant");
            break;
        }
    }

    void onConnected() override {
        spdlog::info("Connected, subscribing to outcomeMetaUpdates...");
        ws_->subscribe(hyperliquid::SubscriptionType::OutcomeMetaUpdates);
    }

    void onDisconnected(bool hasError, const std::string& errMsg) override {
        spdlog::info("Disconnected: hasError={} errMsg={}", hasError, errMsg);
    }

private:
    hyperliquid::WebsocketMessageParser messageParser;
    hyperliquid::WebsocketApi* ws_ = nullptr;
};

int main() {
    hyperliquid::setLogLevel(hyperliquid::LogLevel::Debug);

    hyperliquid::ApiConfig config;
    config.env = hyperliquid::Environment::Testnet;

    OutcomeMetaUpdatesListener listener;
    hyperliquid::WebsocketApi websocket(config, listener);
    listener.setWebsocket(websocket);

    // Recurring markets are created/settled every few minutes on testnet, so a couple of
    // minutes is generally enough to observe real outcomeCreated/outcomeSettled events.
    spdlog::info("Starting websocket, listening for outcomeMetaUpdates for 120s...");
    websocket.start();

    std::this_thread::sleep_for(std::chrono::seconds(120));
    websocket.stop();

    return 0;
}
