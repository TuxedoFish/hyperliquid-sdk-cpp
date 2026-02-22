#pragma once

#include <map>
#include <memory>
#include <string>

#include "WebsocketListener.h"
#include "../types/RequestTypes.h"

namespace hyperliquid
{
    class MarketData
    {
    public:
        explicit MarketData(Environment env, WebsocketListener& listener);
        ~MarketData();

        MarketData(const MarketData&) = delete;
        MarketData& operator=(const MarketData&) = delete;

        void subscribe(SubscriptionType type, const std::map<std::string, std::string>& filters = {});
        void unsubscribe(SubscriptionType type, const std::map<std::string, std::string>& filters = {});

        void start();
        void stop();

    private:
        struct Impl;
        std::unique_ptr<Impl> impl_;
    };
} // namespace hyperliquid
