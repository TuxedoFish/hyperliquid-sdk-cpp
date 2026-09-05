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
    virtual void onL2Book(const L2BookResponse& response, std::optional<uint64_t> correlationId = std::nullopt) {}
    virtual void onCandleSnapshot(const CandleSnapshotResponse& response, std::optional<uint64_t> correlationId = std::nullopt) {}
    virtual void onAllMids(const AllMidsResponse& response, std::optional<uint64_t> correlationId = std::nullopt) {}
    virtual void onOpenOrders(const OpenOrdersResponse& response, std::optional<uint64_t> correlationId = std::nullopt) {}
    virtual void onOrderStatus(const OrderStatusResponse& response, std::optional<uint64_t> correlationId = std::nullopt) {}
    virtual void onUserFills(const UserFillsResponse& response, std::optional<uint64_t> correlationId = std::nullopt) {}
    virtual void onUserFillsByTime(const UserFillsResponse& response, std::optional<uint64_t> correlationId = std::nullopt) {}
    virtual void onClearinghouseState(const ClearinghouseState& response, std::optional<uint64_t> correlationId = std::nullopt) {}
    virtual void onUserRateLimit(const UserRateLimitResponse& response, std::optional<uint64_t> correlationId = std::nullopt) {}
    virtual void onMetaAndAssetCtxs(const MetaAndAssetCtxsResponse& response, std::optional<uint64_t> correlationId = std::nullopt) {}
    virtual void onSpotMetaAndAssetCtxs(const SpotMetaAndAssetCtxsResponse& response, std::optional<uint64_t> correlationId = std::nullopt) {}
    virtual void onSpotClearinghouseState(const SpotClearinghouseStateResponse& response, std::optional<uint64_t> correlationId = std::nullopt) {}
    virtual void onFrontendOpenOrders(const FrontendOpenOrdersResponse& response, std::optional<uint64_t> correlationId = std::nullopt) {}
    virtual void onHistoricalOrders(const HistoricalOrdersResponse& response, std::optional<uint64_t> correlationId = std::nullopt) {}
    virtual void onUserTwapSliceFills(const UserTwapSliceFillsResponse& response, std::optional<uint64_t> correlationId = std::nullopt) {}
    virtual void onSubAccounts(const SubAccountsResponse& response, std::optional<uint64_t> correlationId = std::nullopt) {}
    virtual void onUserFees(const UserFeesResponse& response, std::optional<uint64_t> correlationId = std::nullopt) {}
    virtual void onMaxBuilderFee(const MaxBuilderFeeResponse& response, std::optional<uint64_t> correlationId = std::nullopt) {}
    virtual void onApprovedBuilders(const ApprovedBuildersResponse& response, std::optional<uint64_t> correlationId = std::nullopt) {}
    virtual void onPlaceOrder(const PlaceOrderResponse& response, std::optional<uint64_t> correlationId = std::nullopt) {}
    virtual void onCancelOrder(const CancelOrderResponse& response, std::optional<uint64_t> correlationId = std::nullopt) {}
    virtual void onModifyOrder(const ModifyOrderResponse& response, std::optional<uint64_t> correlationId = std::nullopt) {}
    virtual void onSimpleResponse(const SimpleResponse& response, std::optional<uint64_t> correlationId = std::nullopt) {}
    virtual void onTwapOrder(const TwapOrderResponse& response, std::optional<uint64_t> correlationId = std::nullopt) {}
    virtual void onTwapCancel(const TwapCancelResponse& response, std::optional<uint64_t> correlationId = std::nullopt) {}
    virtual void onDelegations(const DelegationsResponse& response, std::optional<uint64_t> correlationId = std::nullopt) {}
    virtual void onDelegatorSummary(const DelegatorSummaryResponse& response, std::optional<uint64_t> correlationId = std::nullopt) {}
    virtual void onDelegatorHistory(const DelegatorHistoryResponse& response, std::optional<uint64_t> correlationId = std::nullopt) {}
    virtual void onDelegatorRewards(const DelegatorRewardsResponse& response, std::optional<uint64_t> correlationId = std::nullopt) {}
};

}
