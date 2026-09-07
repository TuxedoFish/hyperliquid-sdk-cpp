#pragma once

#include <memory>
#include <string>
#include <string_view>
#include "WebsocketMessageHandler.h"

namespace hyperliquid
{
    // Parses a single raw websocket subscription message and dispatches it, typed, to a
    // WebsocketMessageHandler. Not tied to a specific connection or subscription - WebsocketApi
    // hands you the raw message via WebsocketApiListener::onMessage; construct one of these
    // (reusable across messages) and call crack() yourself to get typed callbacks.
    class WebsocketMessageParser
    {
    public:
        WebsocketMessageParser();
        ~WebsocketMessageParser();

        WebsocketMessageParser(WebsocketMessageParser&&) noexcept;
        WebsocketMessageParser& operator=(WebsocketMessageParser&&) noexcept;
        WebsocketMessageParser(const WebsocketMessageParser&) = delete;
        WebsocketMessageParser& operator=(const WebsocketMessageParser&) = delete;

        void crack(std::string_view message, WebsocketMessageHandler& listener);

    private:
        struct Impl;
        std::unique_ptr<Impl> impl_;
    };
}
