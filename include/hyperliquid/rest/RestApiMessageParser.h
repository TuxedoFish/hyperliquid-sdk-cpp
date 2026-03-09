#pragma once

#include <memory>
#include <string>
#include "../types/RequestTypes.h"
#include "RestEndpointListener.h"

namespace hyperliquid
{
    class RestApiMessageParser
    {
    public:
        explicit RestApiMessageParser(RestEndpointListener& listener);
        ~RestApiMessageParser();

        RestApiMessageParser(RestApiMessageParser&&) noexcept;
        RestApiMessageParser& operator=(RestApiMessageParser&&) noexcept;
        RestApiMessageParser(const RestApiMessageParser&) = delete;
        RestApiMessageParser& operator=(const RestApiMessageParser&) = delete;

        void parse(const std::string& message, RestEndpointType type);

    private:
        struct Impl;
        std::unique_ptr<Impl> impl_;
    };
}
