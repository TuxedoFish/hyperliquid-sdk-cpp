#pragma once

#include <cstdint>
#include <optional>
#include <string>

#include "hyperliquid/types/RequestTypes.h"
#include "hyperliquid/types/ResponseTypes.h"

namespace hyperliquid {

// Callback interface for WebsocketApi: connection lifecycle, raw subscription messages, and
// typed post-response results (one onPostResponse overload per endpoint response type - the
// websocket analogue of RestEndpointListener, dispatched automatically by WebsocketApi itself
// rather than requiring you to call a parser). Subscription messages arrive raw via onMessage;
// pass them to WebsocketMessageParser::crack() yourself (see WebsocketMessageHandler) for typed
// subscription callbacks.
class WebsocketApiListener {
public:
    virtual ~WebsocketApiListener() = default;

    virtual void onMessage(const std::string&) {}
    virtual void onPostResponse(const std::string&, RestEndpointType,
                                std::optional<uint64_t> = std::nullopt) {}
    virtual void onConnected() {}
    virtual void onDisconnected(bool, const std::string&) {}

    virtual void onPostResponse(const SpotMetaResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onPostResponse(const MetaResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onPostResponse(const OutcomeMetaResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onPostResponse(const PerpDexsResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onPostResponse(const PerpAnnotationResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onPostResponse(const PerpCategoriesResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onPostResponse(const PerpConciseAnnotationsResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onPostResponse(const AllPerpMetasResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onPostResponse(const PerpsAtOpenInterestCapResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onPostResponse(const PredictedFundingsResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onPostResponse(const MetaAndAssetCtxsResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onPostResponse(const SpotMetaAndAssetCtxsResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onPostResponse(const SpotClearinghouseStateResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onPostResponse(const FrontendOpenOrdersResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onPostResponse(const HistoricalOrdersResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onPostResponse(const UserTwapSliceFillsResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onPostResponse(const TwapHistoryResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onPostResponse(const ActiveAssetData&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onPostResponse(const SubAccountsResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onPostResponse(const UserFeesResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onPostResponse(const MaxBuilderFeeResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onPostResponse(const ApprovedBuildersResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onPostResponse(const L2BookResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onPostResponse(const CandleSnapshotResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onPostResponse(const AllMidsResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onPostResponse(const OpenOrdersResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onPostResponse(const OrderStatusResponse&, std::optional<uint64_t> = std::nullopt) {}
    // userFills/userFillsByTime share this response shape, so RestEndpointType disambiguates.
    virtual void onPostResponse(RestEndpointType, const UserFillsResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onPostResponse(const ClearinghouseState&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onPostResponse(const VaultDetailsResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onPostResponse(const UserVaultEquitiesResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onPostResponse(const PortfolioResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onPostResponse(const ReferralResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onPostResponse(const UserRoleResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onPostResponse(const UserRateLimitResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onPostResponse(const PerpDexLimitsResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onPostResponse(const PerpDexStatusResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onPostResponse(const PerpDeployAuctionStatusResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onPostResponse(const SettledOutcomeResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onPostResponse(const BorrowLendUserStateResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onPostResponse(const BorrowLendReserveState&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onPostResponse(const AllBorrowLendReserveStatesResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onPostResponse(const SpotDeployStateResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onPostResponse(const SpotPairDeployAuctionStatusResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onPostResponse(const UserDexAbstractionResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onPostResponse(const UserAbstractionResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onPostResponse(const DelegationsResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onPostResponse(const DelegatorSummaryResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onPostResponse(const DelegatorHistoryResponse&, std::optional<uint64_t> = std::nullopt) {}
    virtual void onPostResponse(const DelegatorRewardsResponse&, std::optional<uint64_t> = std::nullopt) {}

    // Every exchange action shares this response shape, so RestEndpointType disambiguates.
    virtual void onPostResponse(RestEndpointType, const SimpleResponse&,
                                std::optional<uint64_t> = std::nullopt) {}
};

}
