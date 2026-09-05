#pragma once

#include "../types/ResponseTypes.h"

namespace hyperliquid {

class WebsocketMessageHandler {
public:
    virtual ~WebsocketMessageHandler() = default;

    virtual void onSubscriptionResponse(const SubscriptionResponse& response) {}
    virtual void onL2Book(const L2BookSnapshot& snapshot) {}
    virtual void onBbo(const BboUpdate& update) {}
    virtual void onTrade(const Trade& trade) {}
    virtual void onCandle(const Candle& candle) {}
    virtual void onAllMidsEntry(const AllMidsEntry& entry) {}
    virtual void onPerpAssetCtx(const PerpAssetCtx& ctx) {}
    virtual void onSpotAssetCtx(const SpotAssetCtx& ctx) {}
    virtual void onOrderUpdate(const OrderUpdate& update) {}
    virtual void onUserFill(const Fill& fill) {}
    virtual void onUserFunding(const UserFunding& funding) {}
    virtual void onLiquidation(const Liquidation& liquidation) {}
    virtual void onNonUserCancel(const NonUserCancel& cancel) {}

    virtual void onUserFundingUpdate(const UserFunding& funding) {}
    virtual void onLedgerUpdate(const LedgerUpdate& update) {}
    virtual void onWebData3(const WebData3Update& update) {}
    virtual void onClearinghouseState(const ClearinghouseStateUpdate& update) {}
    virtual void onOpenOrdersSnapshot(const OpenOrdersUpdate& update) {}

    virtual void onTwapStates(const TwapStatesUpdate& update) {}
    virtual void onNotification(const Notification& notification) {}
    virtual void onUserTwapSliceFill(const TwapSliceFill& fill) {}
    virtual void onUserTwapHistory(const TwapHistoryEntry& entry) {}
    virtual void onActiveAssetData(const ActiveAssetData& data) {}
    virtual void onSpotState(const SpotStateUpdate& update) {}
    virtual void onAllDexsClearinghouseState(const AllDexsClearinghouseStateUpdate& update) {}
    virtual void onAllDexsAssetCtxs(const AllDexsAssetCtxsUpdate& update) {}
    virtual void onFastAssetCtx(const FastAssetCtx& ctx) {}
};

}
