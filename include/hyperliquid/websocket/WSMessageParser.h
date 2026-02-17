#pragma once

#include <memory>
#include <string>
#include "WSMessageHandler.h"

namespace hyperliquid
{
    class WSMessageParser
    {
    public:
        WSMessageParser();
        ~WSMessageParser();

        WSMessageParser(WSMessageParser&&) noexcept;
        WSMessageParser& operator=(WSMessageParser&&) noexcept;
        WSMessageParser(const WSMessageParser&) = delete;
        WSMessageParser& operator=(const WSMessageParser&) = delete;

        void crack(const std::string& message, WSMessageHandler& listener);

    private:
        struct Impl;
        std::unique_ptr<Impl> impl_;
    };
}
