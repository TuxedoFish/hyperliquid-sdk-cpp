#pragma once

#include "../types/ResponseTypes.h"

namespace hyperliquid {

class WebsocketMessageHandler {
public:
    virtual ~WebsocketMessageHandler() = default;

    virtual void onSubscriptionResponse(const SubscriptionResponse&) {}
    virtual void onL2Book(const L2BookSnapshot&) {}
    virtual void onBbo(const BboUpdate&) {}
    virtual void onTrade(const Trade&) {}
    virtual void onCandle(const Candle&) {}
    virtual void onAllMidsEntry(const AllMidsEntry&) {}
    virtual void onPerpAssetCtx(const PerpAssetCtx&) {}
    virtual void onSpotAssetCtx(const SpotAssetCtx&) {}
    virtual void onOrderUpdate(const OrderUpdate&) {}
    virtual void onUserFill(const Fill&) {}
    virtual void onUserFunding(const UserFunding&) {}
    virtual void onLiquidation(const Liquidation&) {}
    virtual void onNonUserCancel(const NonUserCancel&) {}

    virtual void onUserFundingUpdate(const UserFunding&) {}
    virtual void onLedgerUpdate(const LedgerUpdate&) {}
    virtual void onWebData3(const WebData3Update&) {}
    virtual void onClearinghouseState(const ClearinghouseStateUpdate&) {}
    virtual void onOpenOrdersSnapshot(const OpenOrdersUpdate&) {}

    virtual void onTwapStates(const TwapStatesUpdate&) {}
    virtual void onNotification(const Notification&) {}
    virtual void onUserTwapSliceFill(const TwapSliceFill&) {}
    virtual void onUserTwapHistory(const TwapHistoryEntry&) {}
    virtual void onActiveAssetData(const ActiveAssetData&) {}
    virtual void onSpotState(const SpotStateUpdate&) {}
    virtual void onAllDexsClearinghouseState(const AllDexsClearinghouseStateUpdate&) {}
    virtual void onAllDexsAssetCtxs(const AllDexsAssetCtxsUpdate&) {}
    virtual void onFastAssetCtx(const FastAssetCtx&) {}
};

}
