#pragma once

#include <memory>
#include <string>
#include "../types/RequestTypes.h"
#include "InfoEndpointListener.h"

namespace hyperliquid
{
    class RestMessageParser
    {
    public:
        explicit RestMessageParser(InfoEndpointListener& listener);
        ~RestMessageParser();

        RestMessageParser(RestMessageParser&&) noexcept;
        RestMessageParser& operator=(RestMessageParser&&) noexcept;
        RestMessageParser(const RestMessageParser&) = delete;
        RestMessageParser& operator=(const RestMessageParser&) = delete;

        void parse(const std::string& message, InfoEndpointType type);

    private:
        struct Impl;
        std::unique_ptr<Impl> impl_;
    };
}
