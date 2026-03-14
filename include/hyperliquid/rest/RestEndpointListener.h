#pragma once

#include "../types/ResponseTypes.h"

namespace hyperliquid {

class RestEndpointListener {
public:
    virtual ~RestEndpointListener() = default;

    virtual void onSpotMeta(const SpotMetaResponse& response) {}
    virtual void onMeta(const MetaResponse& response) {}
    virtual void onPerpDexs(const PerpDexsResponse& response) {}
    virtual void onPlaceOrder(const PlaceOrderResponse& response) {}
    virtual void onCancelOrder(const CancelOrderResponse& response) {}
    virtual void onModifyOrder(const ModifyOrderResponse& response) {}
};

}
