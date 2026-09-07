#pragma once

#include <memory>
#include <string>
#include "../types/RequestTypes.h"
#include "RestEndpointListener.h"

namespace hyperliquid
{
    // Turns a raw JSON response body into typed response structs. Two ways to use it:
    //  - Construct with no listener and call the parseX(message) methods directly for a single
    //    endpoint's response when you already know the type (stateless, no dispatch).
    //  - Construct with a RestEndpointListener& and call parse(message, type, correlationId) to
    //    have the right parseX method picked for you and the result delivered via the listener's
    //    matching onX callback - this is what both RestApi's async methods and
    //    WebsocketApi's post-response dispatch use internally.
    class RestApiMessageParser
    {
    public:
        RestApiMessageParser();
        explicit RestApiMessageParser(RestEndpointListener& listener);
        ~RestApiMessageParser();

        RestApiMessageParser(RestApiMessageParser&&) noexcept;
        RestApiMessageParser& operator=(RestApiMessageParser&&) noexcept;
        RestApiMessageParser(const RestApiMessageParser&) = delete;
        RestApiMessageParser& operator=(const RestApiMessageParser&) = delete;

        // Parses message as type's response shape and delivers it to the listener passed at
        // construction. The default constructor wires up a no-op listener, so calling this
        // without a real listener silently discards the result - pass one if you want it.
        void parse(const std::string& message, RestEndpointType type,
                   std::optional<uint64_t> correlationId = std::nullopt);

        SpotMetaResponse parseSpotMeta(const std::string& message);
        MetaResponse parseMeta(const std::string& message);
        OutcomeMetaResponse parseOutcomeMeta(const std::string& message);
        SettledOutcomeResponse parseSettledOutcome(const std::string& message);
        PerpDexsResponse parsePerpDexs(const std::string& message);
        PerpsAtOpenInterestCapResponse parsePerpsAtOpenInterestCap(const std::string& message);
        PredictedFundingsResponse parsePredictedFundings(const std::string& message);
        PerpAnnotationResponse parsePerpAnnotation(const std::string& message);
        PerpCategoriesResponse parsePerpCategories(const std::string& message);
        PerpConciseAnnotationsResponse parsePerpConciseAnnotations(const std::string& message);
        AllPerpMetasResponse parseAllPerpMetas(const std::string& message);
        PerpDexLimitsResponse parsePerpDexLimits(const std::string& message);
        PerpDexStatusResponse parsePerpDexStatus(const std::string& message);
        PerpDeployAuctionStatusResponse parsePerpDeployAuctionStatus(const std::string& message);
        UserFundingLedgerUpdateResponse parseUserFunding(const std::string& message);
        UserFundingLedgerUpdateResponse parseUserNonFundingLedgerUpdates(const std::string& message);
        FundingHistoryResponse parseFundingHistory(const std::string& message);
        L2BookResponse parseL2Book(const std::string& message);
        CandleSnapshotResponse parseCandleSnapshot(const std::string& message);
        RecentTradesResponse parseRecentTrades(const std::string& message);
        AllMidsResponse parseAllMids(const std::string& message);
        OpenOrdersResponse parseOpenOrders(const std::string& message);
        OrderStatusResponse parseOrderStatus(const std::string& message);
        UserFillsResponse parseUserFills(const std::string& message);
        UserFillsResponse parseUserFillsByTime(const std::string& message);
        ClearinghouseState parseClearinghouseState(const std::string& message);
        UserRateLimitResponse parseUserRateLimit(const std::string& message);
        MetaAndAssetCtxsResponse parseMetaAndAssetCtxs(const std::string& message);
        SpotMetaAndAssetCtxsResponse parseSpotMetaAndAssetCtxs(const std::string& message);
        SpotClearinghouseStateResponse parseSpotClearinghouseState(const std::string& message);
        SpotDeployStateResponse parseSpotDeployState(const std::string& message);
        SpotPairDeployAuctionStatusResponse parseSpotPairDeployAuctionStatus(const std::string& message);
        FrontendOpenOrdersResponse parseFrontendOpenOrders(const std::string& message);
        HistoricalOrdersResponse parseHistoricalOrders(const std::string& message);
        UserTwapSliceFillsResponse parseUserTwapSliceFills(const std::string& message);
        SubAccountsResponse parseSubAccounts(const std::string& message);
        UserFeesResponse parseUserFees(const std::string& message);
        MaxBuilderFeeResponse parseMaxBuilderFee(const std::string& message);
        ApprovedBuildersResponse parseApprovedBuilders(const std::string& message);
        VaultDetailsResponse parseVaultDetails(const std::string& message);
        UserVaultEquitiesResponse parseUserVaultEquities(const std::string& message);
        PortfolioResponse parsePortfolio(const std::string& message);
        ReferralResponse parseReferral(const std::string& message);
        UserRoleResponse parseUserRole(const std::string& message);
        BorrowLendUserStateResponse parseBorrowLendUserState(const std::string& message);
        BorrowLendReserveState parseBorrowLendReserveState(const std::string& message);
        AllBorrowLendReserveStatesResponse parseAllBorrowLendReserveStates(const std::string& message);
        PlaceOrderResponse parsePlaceOrder(const std::string& message);
        CancelOrderResponse parseCancelOrder(const std::string& message);
        ModifyOrderResponse parseModifyOrder(const std::string& message);
        SimpleResponse parseSimpleResponse(const std::string& message);
        TwapOrderResponse parseTwapOrder(const std::string& message);
        TwapCancelResponse parseTwapCancel(const std::string& message);
        DelegationsResponse parseDelegations(const std::string& message);
        DelegatorSummaryResponse parseDelegatorSummary(const std::string& message);
        DelegatorHistoryResponse parseDelegatorHistory(const std::string& message);
        DelegatorRewardsResponse parseDelegatorRewards(const std::string& message);
        UserDexAbstractionResponse parseUserDexAbstractionState(const std::string& message);
        UserAbstractionResponse parseUserAbstraction(const std::string& message);

    private:
        struct Impl;
        std::unique_ptr<Impl> impl_;
    };
}
