#pragma once

#include <map>
#include <memory>
#include <string>

#include "WebsocketApiListener.h"
#include "../types/RequestTypes.h"
#include "hyperliquid/config/Config.h"

namespace hyperliquid
{
    class WebsocketApi
    {
    public:
        explicit WebsocketApi(ApiConfig& config, WebsocketApiListener& listener);
        ~WebsocketApi();

        WebsocketApi(const WebsocketApi&) = delete;
        WebsocketApi& operator=(const WebsocketApi&) = delete;

        // Subscription requests
        void subscribe(SubscriptionType type, const std::map<std::string, std::string>& filters = {});
        void unsubscribe(SubscriptionType type, const std::map<std::string, std::string>& filters = {});

        // Post requests over websocket
        void spotMeta();
        void meta(const std::optional<std::string>& dex = std::nullopt);
        void perpDexs();
        void placeOrder(const std::vector<OrderRequest>& orders,
                               Grouping grouping,
                               const std::optional<Builder>& builder = std::nullopt);
        void cancelOrder(const std::vector<CancelRequest>& cancels);
        void cancelOrderByCloid(const std::vector<CancelByCloidRequest>& cancels);
        void modifyOrder(const ModifyRequest& modify);
        void batchModifyOrder(const std::vector<ModifyRequest>& modifies);

        void start();
        void stop();

    private:
        struct Impl;
        std::unique_ptr<Impl> impl_;
    };
}
