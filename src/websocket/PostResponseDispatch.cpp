#include "PostResponseDispatch.h"

#include <simdjson.h>

#include "config/Logger.h"
#include "hyperliquid/rest/RestApiMessageParser.h"
#include "hyperliquid/websocket/WebsocketApiListener.h"

namespace hyperliquid::internal
{
    namespace
    {
        // Info-read post responses are wire-wrapped as {"type":"<endpoint>","data":{...}} - the
        // "data" sub-object is exactly what RestApi's parseX functions expect (they parse the
        // same shape RestApi's synchronous HTTP responses use). Extract just that sub-object here
        // and hand it to the matching parser + typed callback.
        void dispatchInfoPostResponse(RestEndpointType type, const std::string& payloadJson,
                                      std::optional<uint64_t> correlationId, WebsocketApiListener& listener)
        {
            std::string dataJson;
            try
            {
                simdjson::ondemand::parser parser;
                simdjson::padded_string padded(payloadJson.data(), payloadJson.size());
                auto doc = parser.iterate(padded);
                dataJson = std::string(simdjson::to_json_string(doc["data"]).value());
            }
            catch (const simdjson::simdjson_error& e)
            {
                getLogger()->error("PostResponseDispatch: failed to extract typed payload for {}: {}\n  raw: {}",
                                   toString(type), e.what(), payloadJson);
                return;
            }

            static thread_local RestApiMessageParser parser;

            switch (type)
            {
            case RestEndpointType::SpotMeta:
                listener.onSpotMetaPostResponse(parser.parseSpotMeta(dataJson), correlationId);
                break;
            case RestEndpointType::Meta:
                listener.onMetaPostResponse(parser.parseMeta(dataJson), correlationId);
                break;
            case RestEndpointType::OutcomeMeta:
                listener.onOutcomeMetaPostResponse(parser.parseOutcomeMeta(dataJson), correlationId);
                break;
            case RestEndpointType::PerpDexs:
                listener.onPerpDexsPostResponse(parser.parsePerpDexs(dataJson), correlationId);
                break;
            case RestEndpointType::PerpAnnotation:
                listener.onPerpAnnotationPostResponse(parser.parsePerpAnnotation(dataJson), correlationId);
                break;
            case RestEndpointType::PerpCategories:
                listener.onPerpCategoriesPostResponse(parser.parsePerpCategories(dataJson), correlationId);
                break;
            case RestEndpointType::PerpConciseAnnotations:
                listener.onPerpConciseAnnotationsPostResponse(parser.parsePerpConciseAnnotations(dataJson), correlationId);
                break;
            case RestEndpointType::AllPerpMetas:
                listener.onAllPerpMetasPostResponse(parser.parseAllPerpMetas(dataJson), correlationId);
                break;
            case RestEndpointType::PerpsAtOpenInterestCap:
                listener.onPerpsAtOpenInterestCapPostResponse(parser.parsePerpsAtOpenInterestCap(dataJson), correlationId);
                break;
            case RestEndpointType::PredictedFundings:
                listener.onPredictedFundingsPostResponse(parser.parsePredictedFundings(dataJson), correlationId);
                break;
            case RestEndpointType::MetaAndAssetCtxs:
                listener.onMetaAndAssetCtxsPostResponse(parser.parseMetaAndAssetCtxs(dataJson), correlationId);
                break;
            case RestEndpointType::SpotMetaAndAssetCtxs:
                listener.onSpotMetaAndAssetCtxsPostResponse(parser.parseSpotMetaAndAssetCtxs(dataJson), correlationId);
                break;
            case RestEndpointType::SpotClearinghouseState:
                listener.onSpotClearinghouseStatePostResponse(parser.parseSpotClearinghouseState(dataJson), correlationId);
                break;
            case RestEndpointType::FrontendOpenOrders:
                listener.onFrontendOpenOrdersPostResponse(parser.parseFrontendOpenOrders(dataJson), correlationId);
                break;
            case RestEndpointType::HistoricalOrders:
                listener.onHistoricalOrdersPostResponse(parser.parseHistoricalOrders(dataJson), correlationId);
                break;
            case RestEndpointType::UserTwapSliceFills:
                listener.onUserTwapSliceFillsPostResponse(parser.parseUserTwapSliceFills(dataJson), correlationId);
                break;
            case RestEndpointType::SubAccounts:
                listener.onSubAccountsPostResponse(parser.parseSubAccounts(dataJson), correlationId);
                break;
            case RestEndpointType::UserFees:
                listener.onUserFeesPostResponse(parser.parseUserFees(dataJson), correlationId);
                break;
            case RestEndpointType::MaxBuilderFee:
                listener.onMaxBuilderFeePostResponse(parser.parseMaxBuilderFee(dataJson), correlationId);
                break;
            case RestEndpointType::ApprovedBuilders:
                listener.onApprovedBuildersPostResponse(parser.parseApprovedBuilders(dataJson), correlationId);
                break;
            case RestEndpointType::L2Book:
                listener.onL2BookPostResponse(parser.parseL2Book(dataJson), correlationId);
                break;
            case RestEndpointType::CandleSnapshot:
                listener.onCandleSnapshotPostResponse(parser.parseCandleSnapshot(dataJson), correlationId);
                break;
            case RestEndpointType::AllMids:
                listener.onAllMidsPostResponse(parser.parseAllMids(dataJson), correlationId);
                break;
            case RestEndpointType::OpenOrders:
                listener.onOpenOrdersPostResponse(parser.parseOpenOrders(dataJson), correlationId);
                break;
            case RestEndpointType::OrderStatus:
                listener.onOrderStatusPostResponse(parser.parseOrderStatus(dataJson), correlationId);
                break;
            case RestEndpointType::UserFills:
                listener.onUserFillsPostResponse(parser.parseUserFills(dataJson), correlationId);
                break;
            case RestEndpointType::UserFillsByTime:
                listener.onUserFillsByTimePostResponse(parser.parseUserFillsByTime(dataJson), correlationId);
                break;
            case RestEndpointType::ClearinghouseState:
                listener.onClearinghouseStatePostResponse(parser.parseClearinghouseState(dataJson), correlationId);
                break;
            case RestEndpointType::VaultDetails:
                listener.onVaultDetailsPostResponse(parser.parseVaultDetails(dataJson), correlationId);
                break;
            case RestEndpointType::UserVaultEquities:
                listener.onUserVaultEquitiesPostResponse(parser.parseUserVaultEquities(dataJson), correlationId);
                break;
            case RestEndpointType::Portfolio:
                listener.onPortfolioPostResponse(parser.parsePortfolio(dataJson), correlationId);
                break;
            case RestEndpointType::Referral:
                listener.onReferralPostResponse(parser.parseReferral(dataJson), correlationId);
                break;
            case RestEndpointType::UserRole:
                listener.onUserRolePostResponse(parser.parseUserRole(dataJson), correlationId);
                break;
            case RestEndpointType::UserRateLimit:
                listener.onUserRateLimitPostResponse(parser.parseUserRateLimit(dataJson), correlationId);
                break;
            case RestEndpointType::PerpDexLimits:
                listener.onPerpDexLimitsPostResponse(parser.parsePerpDexLimits(dataJson), correlationId);
                break;
            case RestEndpointType::PerpDexStatus:
                listener.onPerpDexStatusPostResponse(parser.parsePerpDexStatus(dataJson), correlationId);
                break;
            case RestEndpointType::PerpDeployAuctionStatus:
                listener.onPerpDeployAuctionStatusPostResponse(parser.parsePerpDeployAuctionStatus(dataJson), correlationId);
                break;
            case RestEndpointType::SettledOutcome:
                listener.onSettledOutcomePostResponse(parser.parseSettledOutcome(dataJson), correlationId);
                break;
            case RestEndpointType::BorrowLendUserState:
                listener.onBorrowLendUserStatePostResponse(parser.parseBorrowLendUserState(dataJson), correlationId);
                break;
            case RestEndpointType::BorrowLendReserveState:
                listener.onBorrowLendReserveStatePostResponse(parser.parseBorrowLendReserveState(dataJson), correlationId);
                break;
            case RestEndpointType::AllBorrowLendReserveStates:
                listener.onAllBorrowLendReserveStatesPostResponse(parser.parseAllBorrowLendReserveStates(dataJson), correlationId);
                break;
            case RestEndpointType::SpotDeployState:
                listener.onSpotDeployStatePostResponse(parser.parseSpotDeployState(dataJson), correlationId);
                break;
            case RestEndpointType::SpotPairDeployAuctionStatus:
                listener.onSpotPairDeployAuctionStatusPostResponse(parser.parseSpotPairDeployAuctionStatus(dataJson), correlationId);
                break;
            case RestEndpointType::Delegations:
                listener.onDelegationsPostResponse(parser.parseDelegations(dataJson), correlationId);
                break;
            case RestEndpointType::DelegatorSummary:
                listener.onDelegatorSummaryPostResponse(parser.parseDelegatorSummary(dataJson), correlationId);
                break;
            case RestEndpointType::DelegatorHistory:
                listener.onDelegatorHistoryPostResponse(parser.parseDelegatorHistory(dataJson), correlationId);
                break;
            case RestEndpointType::DelegatorRewards:
                listener.onDelegatorRewardsPostResponse(parser.parseDelegatorRewards(dataJson), correlationId);
                break;
            default:
                getLogger()->error("PostResponseDispatch: unhandled info RestEndpointType: {}", toString(type));
                break;
            }
        }

        void dispatchExchangeActionPostResponse(RestEndpointType type, const std::string& payloadJson,
                                                std::optional<uint64_t> correlationId, WebsocketApiListener& listener)
        {
            static thread_local RestApiMessageParser parser;
            listener.onExchangeActionPostResponse(type, parser.parseSimpleResponse(payloadJson), correlationId);
        }
    }

    void handlePostChannelMessage(const std::string& rawMessage,
                                  std::unordered_map<uint64_t, PostRequestInfo>& postRequestInfo,
                                  WebsocketApiListener& listener)
    {
        simdjson::ondemand::parser parser;
        simdjson::padded_string padded(rawMessage.data(), rawMessage.size());
        auto doc = parser.iterate(padded);

        auto data = doc["data"].get_object().value();
        uint64_t id = data["id"].get_uint64().value();
        auto payload = simdjson::to_json_string(data["response"]["payload"]);

        auto it = postRequestInfo.find(id);
        if (it == postRequestInfo.end())
        {
            getLogger()->error("post response with unknown id: {}", id);
            listener.onMessage(rawMessage);
            return;
        }

        auto info = it->second;
        postRequestInfo.erase(it);

        std::string payloadStr(payload.value());
        listener.onPostResponse(payloadStr, info.type, info.correlationId);

        if (isAuthenticated(info.type))
            dispatchExchangeActionPostResponse(info.type, payloadStr, info.correlationId, listener);
        else
            dispatchInfoPostResponse(info.type, payloadStr, info.correlationId, listener);
    }
}
