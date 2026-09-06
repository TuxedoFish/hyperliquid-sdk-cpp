#pragma once

#include <cstdint>
#include <optional>

#include "../types/ResponseTypes.h"

namespace hyperliquid {

class RestEndpointListener {
public:
    virtual ~RestEndpointListener() = default;

    virtual void onSpotMeta(const SpotMetaResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onMeta(const MetaResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onOutcomeMeta(const OutcomeMetaResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onSettledOutcome(const SettledOutcomeResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onPerpDexs(const PerpDexsResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onPerpDexLimits(const PerpDexLimitsResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onPerpDexStatus(const PerpDexStatusResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onPerpDeployAuctionStatus(const PerpDeployAuctionStatusResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onPerpsAtOpenInterestCap(const PerpsAtOpenInterestCapResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onPredictedFundings(const PredictedFundingsResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onPerpAnnotation(const PerpAnnotationResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onPerpCategories(const PerpCategoriesResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onPerpConciseAnnotations(const PerpConciseAnnotationsResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onAllPerpMetas(const AllPerpMetasResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onL2Book(const L2BookResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onCandleSnapshot(const CandleSnapshotResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onAllMids(const AllMidsResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onOpenOrders(const OpenOrdersResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onOrderStatus(const OrderStatusResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onUserFills(const UserFillsResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onUserFillsByTime(const UserFillsResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onClearinghouseState(const ClearinghouseState&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onUserRateLimit(const UserRateLimitResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onMetaAndAssetCtxs(const MetaAndAssetCtxsResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onSpotMetaAndAssetCtxs(const SpotMetaAndAssetCtxsResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onSpotClearinghouseState(const SpotClearinghouseStateResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onSpotDeployState(const SpotDeployStateResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onSpotPairDeployAuctionStatus(const SpotPairDeployAuctionStatusResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onFrontendOpenOrders(const FrontendOpenOrdersResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onHistoricalOrders(const HistoricalOrdersResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onUserTwapSliceFills(const UserTwapSliceFillsResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onSubAccounts(const SubAccountsResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onUserFees(const UserFeesResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onMaxBuilderFee(const MaxBuilderFeeResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onApprovedBuilders(const ApprovedBuildersResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onVaultDetails(const VaultDetailsResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onUserVaultEquities(const UserVaultEquitiesResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onPortfolio(const PortfolioResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onReferral(const ReferralResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onUserRole(const UserRoleResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onPlaceOrder(const PlaceOrderResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onCancelOrder(const CancelOrderResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onModifyOrder(const ModifyOrderResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onSimpleResponse(const SimpleResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onTwapOrder(const TwapOrderResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onTwapCancel(const TwapCancelResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onDelegations(const DelegationsResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onDelegatorSummary(const DelegatorSummaryResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onDelegatorHistory(const DelegatorHistoryResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onDelegatorRewards(const DelegatorRewardsResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onBorrowLendUserState(const BorrowLendUserStateResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onBorrowLendReserveState(const BorrowLendReserveState&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onAllBorrowLendReserveStates(const AllBorrowLendReserveStatesResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onUserDexAbstractionState(const UserDexAbstractionResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onUserAbstraction(const UserAbstractionResponse&, std::optional<uint64_t> = std::nullopt) {}
};

}
