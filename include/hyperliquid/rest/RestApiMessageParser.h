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

        void parse(const std::string& message, RestEndpointType type,
                   std::optional<uint64_t> correlationId = std::nullopt);

        SpotMetaResponse parseSpotMeta(const std::string& message);
        MetaResponse parseMeta(const std::string& message);
        OutcomeMetaResponse parseOutcomeMeta(const std::string& message);
        PerpDexsResponse parsePerpDexs(const std::string& message);
        L2BookResponse parseL2Book(const std::string& message);
        CandleSnapshotResponse parseCandleSnapshot(const std::string& message);
        AllMidsResponse parseAllMids(const std::string& message);
        OpenOrdersResponse parseOpenOrders(const std::string& message);
        OrderStatusResponse parseOrderStatus(const std::string& message);
        UserFillsResponse parseUserFills(const std::string& message);
        UserFillsResponse parseUserFillsByTime(const std::string& message);
        ClearinghouseState parseClearinghouseState(const std::string& message);
        PlaceOrderResponse parsePlaceOrder(const std::string& message);
        CancelOrderResponse parseCancelOrder(const std::string& message);
        ModifyOrderResponse parseModifyOrder(const std::string& message);
        SimpleResponse parseSimpleResponse(const std::string& message);

    private:
        struct Impl;
        std::unique_ptr<Impl> impl_;
    };
}
