#pragma once

#include <cstdint>
#include <optional>
#include <string>

#include "hyperliquid/types/RequestTypes.h"
#include "hyperliquid/types/ResponseTypes.h"

namespace hyperliquid {

class WebsocketApiListener {
public:
    virtual ~WebsocketApiListener() = default;

    virtual void onMessage(const std::string&) {}
    virtual void onPostResponse(const std::string&, RestEndpointType,
                                std::optional<uint64_t> = std::nullopt) {}
    virtual void onConnected() {}
    virtual void onDisconnected(bool, const std::string&) {}

    // Typed post-response callbacks - additive to onPostResponse above, not a replacement.
    //
    // Info reads (unauthenticated): one typed callback per endpoint, each carrying the same
    // response struct RestApi's synchronous call for that endpoint returns.
    virtual void onSpotMetaPostResponse(const SpotMetaResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onMetaPostResponse(const MetaResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onOutcomeMetaPostResponse(const OutcomeMetaResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onPerpDexsPostResponse(const PerpDexsResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onPerpAnnotationPostResponse(const PerpAnnotationResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onPerpCategoriesPostResponse(const PerpCategoriesResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onPerpConciseAnnotationsPostResponse(const PerpConciseAnnotationsResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onAllPerpMetasPostResponse(const AllPerpMetasResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onPerpsAtOpenInterestCapPostResponse(const PerpsAtOpenInterestCapResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onPredictedFundingsPostResponse(const PredictedFundingsResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onMetaAndAssetCtxsPostResponse(const MetaAndAssetCtxsResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onSpotMetaAndAssetCtxsPostResponse(const SpotMetaAndAssetCtxsResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onSpotClearinghouseStatePostResponse(const SpotClearinghouseStateResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onFrontendOpenOrdersPostResponse(const FrontendOpenOrdersResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onHistoricalOrdersPostResponse(const HistoricalOrdersResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onUserTwapSliceFillsPostResponse(const UserTwapSliceFillsResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onSubAccountsPostResponse(const SubAccountsResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onUserFeesPostResponse(const UserFeesResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onMaxBuilderFeePostResponse(const MaxBuilderFeeResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onApprovedBuildersPostResponse(const ApprovedBuildersResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onL2BookPostResponse(const L2BookResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onCandleSnapshotPostResponse(const CandleSnapshotResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onAllMidsPostResponse(const AllMidsResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onOpenOrdersPostResponse(const OpenOrdersResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onOrderStatusPostResponse(const OrderStatusResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onUserFillsPostResponse(const UserFillsResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onUserFillsByTimePostResponse(const UserFillsResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onClearinghouseStatePostResponse(const ClearinghouseState&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onVaultDetailsPostResponse(const VaultDetailsResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onUserVaultEquitiesPostResponse(const UserVaultEquitiesResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onPortfolioPostResponse(const PortfolioResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onReferralPostResponse(const ReferralResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onUserRolePostResponse(const UserRoleResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onUserRateLimitPostResponse(const UserRateLimitResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onPerpDexLimitsPostResponse(const PerpDexLimitsResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onPerpDexStatusPostResponse(const PerpDexStatusResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onPerpDeployAuctionStatusPostResponse(const PerpDeployAuctionStatusResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onSettledOutcomePostResponse(const SettledOutcomeResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onBorrowLendUserStatePostResponse(const BorrowLendUserStateResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onBorrowLendReserveStatePostResponse(const BorrowLendReserveState&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onAllBorrowLendReserveStatesPostResponse(const AllBorrowLendReserveStatesResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onSpotDeployStatePostResponse(const SpotDeployStateResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onSpotPairDeployAuctionStatusPostResponse(const SpotPairDeployAuctionStatusResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onDelegationsPostResponse(const DelegationsResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onDelegatorSummaryPostResponse(const DelegatorSummaryResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onDelegatorHistoryPostResponse(const DelegatorHistoryResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onDelegatorRewardsPostResponse(const DelegatorRewardsResponse&, std::optional<uint64_t> = std::nullopt) {}

    // Exchange actions (authenticated): every exchange endpoint shares the same response shape
    // (SimpleResponse), so one callback covers all of them - the RestEndpointType tells the
    // caller which action it was.
    virtual void onExchangeActionPostResponse(RestEndpointType, const SimpleResponse&,
                                              std::optional<uint64_t> = std::nullopt) {}
};

}
