#pragma once

#include "../types/ResponseTypes.h"

namespace hyperliquid {

class WebsocketMessageHandler {
public:
    virtual ~WebsocketMessageHandler() = default;

    virtual void onSubscriptionResponse(const SubscriptionResponse& response) {}
    virtual void onL2BookLevel(const L2BookUpdate& book, const PriceLevel& level) {}
    virtual void onBbo(const BboUpdate& update) {}
    virtual void onTrade(const Trade& trade) {}
    virtual void onCandle(const Candle& candle) {}
    virtual void onAllMidsEntry(const AllMidsEntry& entry) {}
    virtual void onPerpAssetCtx(const PerpAssetCtx& ctx) {}
    virtual void onSpotAssetCtx(const SpotAssetCtx& ctx) {}
    virtual void onOrderUpdate(const OrderUpdate& update, bool isSnapshot = false) {}
    virtual void onUserFill(const Fill& fill, bool isSnapshot = false) {}
    virtual void onUserFunding(const UserFunding& funding) {}
    virtual void onLiquidation(const Liquidation& liquidation) {}
    virtual void onNonUserCancel(const NonUserCancel& cancel) {}
};

}
