#pragma once

#include <map>
#include <memory>
#include <string>

#include "WebsocketApiListener.h"
#include "../types/RequestTypes.h"

namespace hyperliquid
{
    class WebsocketApi
    {
    public:
        explicit WebsocketApi(Environment env, WebsocketApiListener& listener);
        ~WebsocketApi();

        WebsocketApi(const WebsocketApi&) = delete;
        WebsocketApi& operator=(const WebsocketApi&) = delete;

        //
        void subscribe(SubscriptionType type, const std::map<std::string, std::string>& filters = {});
        void unsubscribe(SubscriptionType type, const std::map<std::string, std::string>& filters = {});

        // Post requests over websocket
        void sendRequest(RestEndpointType type, const std::map<std::string, std::string>& params = {});

        void start();
        void stop();

    private:
        struct Impl;
        std::unique_ptr<Impl> impl_;
    };
} // namespace hyperliquid
