#pragma once

#include <memory>
#include <string>
#include <string_view>
#include "WebsocketMessageHandler.h"

namespace hyperliquid
{
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
        void reset();

    private:
        struct Impl;
        std::unique_ptr<Impl> impl_;
    };
}
