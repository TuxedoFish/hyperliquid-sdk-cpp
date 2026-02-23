#pragma once

#include "../types/ResponseTypes.h"

namespace hyperliquid {

class WSMessageHandler {
public:
    virtual ~WSMessageHandler() = default;

    virtual void onSubscriptionResponse(const SubscriptionResponse& response) {}
    virtual void onL2BookLevel(const L2BookUpdate& book, const PriceLevel& level) {}
    virtual void onBbo(const BboUpdate& update) {}
    virtual void onTrade(const Trade& trade) {}
    virtual void onCandle(const Candle& candle) {}
    virtual void onAllMidsEntry(const AllMidsEntry& entry) {}
    virtual void onPerpAssetCtx(const PerpAssetCtx& ctx) {}
    virtual void onSpotAssetCtx(const SpotAssetCtx& ctx) {}
};

}
