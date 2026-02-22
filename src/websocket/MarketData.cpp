#include "hyperliquid/websocket/MarketData.h"
#include "hyperliquid/websocket/WebsocketListener.h"
#include "WSRunner.h"
#include <nlohmann/json.hpp>
#include <thread>

namespace hyperliquid {

struct MarketData::Impl : internal::WSListener {
    internal::WSRunner ws;
    WebsocketListener& listener;
    bool stopping = false;
    std::thread thread;

    Impl(Environment env, WebsocketListener& listener)
        : ws(toWsEndpoint(env).host, toWsEndpoint(env).port, toWsEndpoint(env).path, *this), listener(listener) {}

    ~Impl() {
        ws.stop();
        if (thread.joinable()) thread.join();
    }

    void onWsMessage(std::string& message) override {
        if (stopping) return;
        listener.onMessage(message);
    }

    void onWsConnected() override {
        listener.onConnected();
    }

    void onWsDisconnected(bool hasError, const std::string& errMsg) override {
        listener.onDisconnected(hasError, errMsg);
    }
};

MarketData::MarketData(Environment env, WebsocketListener& listener) : impl_(std::make_unique<Impl>(env, listener)) {}

MarketData::~MarketData() = default;

void MarketData::subscribe(const SubscriptionType type, const std::map<std::string, std::string>& filters)
{
    nlohmann::json subscribeMsg = {
        {"method", "subscribe"},
        {"subscription", {
                    {"type", toString(type)},
        }}
    };
    for (const auto& [key, value] : filters) {
        subscribeMsg["subscription"][key] = value;
    }
    impl_->ws.send(subscribeMsg.dump());
}

void MarketData::unsubscribe(const SubscriptionType type, const std::map<std::string, std::string>& filters)
{
    nlohmann::json unsubscribeMsg = {
        {"method", "unsubscribe"},
        {"subscription", {
                        {"type", toString(type)},
            }}
    };
    for (const auto& [key, value] : filters) {
        unsubscribeMsg["subscription"][key] = value;
    }
    impl_->ws.send(unsubscribeMsg.dump());
}

void MarketData::start() {
    impl_->thread = std::thread([this]() {
        impl_->ws.start();
    });
}

void MarketData::stop() {
    impl_->stopping = true;
    impl_->ws.stop();
    if (impl_->thread.joinable()) impl_->thread.join();
}

}
