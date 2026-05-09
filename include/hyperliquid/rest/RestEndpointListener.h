#pragma once

#include <cstdint>
#include <optional>

#include "../types/ResponseTypes.h"

namespace hyperliquid {

class RestEndpointListener {
public:
    virtual ~RestEndpointListener() = default;

    virtual void onSpotMeta(const SpotMetaResponse& response, std::optional<uint64_t> correlationId = std::nullopt) {}
    virtual void onMeta(const MetaResponse& response, std::optional<uint64_t> correlationId = std::nullopt) {}
    virtual void onOutcomeMeta(const OutcomeMetaResponse& response, std::optional<uint64_t> correlationId = std::nullopt) {}
    virtual void onPerpDexs(const PerpDexsResponse& response, std::optional<uint64_t> correlationId = std::nullopt) {}
    virtual void onPlaceOrder(const PlaceOrderResponse& response, std::optional<uint64_t> correlationId = std::nullopt) {}
    virtual void onCancelOrder(const CancelOrderResponse& response, std::optional<uint64_t> correlationId = std::nullopt) {}
    virtual void onModifyOrder(const ModifyOrderResponse& response, std::optional<uint64_t> correlationId = std::nullopt) {}
};

}
