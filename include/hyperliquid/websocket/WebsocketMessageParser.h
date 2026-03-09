#pragma once

#include <memory>
#include <string>
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

        void crack(const std::string& message, WebsocketMessageHandler& listener);

    private:
        struct Impl;
        std::unique_ptr<Impl> impl_;
    };
}
