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
        RestApiMessageParser();
        explicit RestApiMessageParser(RestEndpointListener& listener);
        ~RestApiMessageParser();

        RestApiMessageParser(RestApiMessageParser&&) noexcept;
        RestApiMessageParser& operator=(RestApiMessageParser&&) noexcept;
        RestApiMessageParser(const RestApiMessageParser&) = delete;
        RestApiMessageParser& operator=(const RestApiMessageParser&) = delete;

        void parse(const std::string& message, RestEndpointType type);

        SpotMetaResponse parseSpotMeta(const std::string& message);
        MetaResponse parseMeta(const std::string& message);
        PerpDexsResponse parsePerpDexs(const std::string& message);
        PlaceOrderResponse parsePlaceOrder(const std::string& message);
        CancelOrderResponse parseCancelOrder(const std::string& message);
        ModifyOrderResponse parseModifyOrder(const std::string& message);

    private:
        struct Impl;
        std::unique_ptr<Impl> impl_;
    };
}
