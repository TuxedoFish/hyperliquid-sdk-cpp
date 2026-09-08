#include "PostResponseDispatch.h"

#include <simdjson.h>

#include "config/Logger.h"
#include "hyperliquid/rest/RestApiMessageParser.h"
#include "hyperliquid/websocket/WebsocketApiListener.h"

namespace hyperliquid::internal
{
    namespace
    {
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
                listener.onPostResponse(parser.parseSpotMeta(dataJson), correlationId);
                break;
            case RestEndpointType::Meta:
                listener.onPostResponse(parser.parseMeta(dataJson), correlationId);
                break;
            case RestEndpointType::OutcomeMeta:
                listener.onPostResponse(parser.parseOutcomeMeta(dataJson), correlationId);
                break;
            case RestEndpointType::PerpDexs:
                listener.onPostResponse(parser.parsePerpDexs(dataJson), correlationId);
                break;
            case RestEndpointType::PerpAnnotation:
                listener.onPostResponse(parser.parsePerpAnnotation(dataJson), correlationId);
                break;
            case RestEndpointType::PerpCategories:
                listener.onPostResponse(parser.parsePerpCategories(dataJson), correlationId);
                break;
            case RestEndpointType::PerpConciseAnnotations:
                listener.onPostResponse(parser.parsePerpConciseAnnotations(dataJson), correlationId);
                break;
            case RestEndpointType::AllPerpMetas:
                listener.onPostResponse(parser.parseAllPerpMetas(dataJson), correlationId);
                break;
            case RestEndpointType::PerpsAtOpenInterestCap:
                listener.onPostResponse(parser.parsePerpsAtOpenInterestCap(dataJson), correlationId);
                break;
            case RestEndpointType::PredictedFundings:
                listener.onPostResponse(parser.parsePredictedFundings(dataJson), correlationId);
                break;
            case RestEndpointType::MetaAndAssetCtxs:
                listener.onPostResponse(parser.parseMetaAndAssetCtxs(dataJson), correlationId);
                break;
            case RestEndpointType::SpotMetaAndAssetCtxs:
                listener.onPostResponse(parser.parseSpotMetaAndAssetCtxs(dataJson), correlationId);
                break;
            case RestEndpointType::SpotClearinghouseState:
                listener.onPostResponse(parser.parseSpotClearinghouseState(dataJson), correlationId);
                break;
            case RestEndpointType::FrontendOpenOrders:
                listener.onPostResponse(parser.parseFrontendOpenOrders(dataJson), correlationId);
                break;
            case RestEndpointType::HistoricalOrders:
                listener.onPostResponse(parser.parseHistoricalOrders(dataJson), correlationId);
                break;
            case RestEndpointType::UserTwapSliceFills:
                listener.onPostResponse(parser.parseUserTwapSliceFills(dataJson), correlationId);
                break;
            case RestEndpointType::UserTwapSliceFillsByTime:
                listener.onPostResponse(parser.parseUserTwapSliceFillsByTime(dataJson), correlationId);
                break;
            case RestEndpointType::TwapHistory:
                listener.onPostResponse(parser.parseTwapHistory(dataJson), correlationId);
                break;
            case RestEndpointType::ActiveAssetData:
                listener.onPostResponse(parser.parseActiveAssetData(dataJson), correlationId);
                break;
            case RestEndpointType::SubAccounts:
                listener.onPostResponse(parser.parseSubAccounts(dataJson), correlationId);
                break;
            case RestEndpointType::UserFees:
                listener.onPostResponse(parser.parseUserFees(dataJson), correlationId);
                break;
            case RestEndpointType::MaxBuilderFee:
                listener.onPostResponse(parser.parseMaxBuilderFee(dataJson), correlationId);
                break;
            case RestEndpointType::ApprovedBuilders:
                listener.onPostResponse(parser.parseApprovedBuilders(dataJson), correlationId);
                break;
            case RestEndpointType::L2Book:
                listener.onPostResponse(parser.parseL2Book(dataJson), correlationId);
                break;
            case RestEndpointType::CandleSnapshot:
                listener.onPostResponse(parser.parseCandleSnapshot(dataJson), correlationId);
                break;
            case RestEndpointType::AllMids:
                listener.onPostResponse(parser.parseAllMids(dataJson), correlationId);
                break;
            case RestEndpointType::OpenOrders:
                listener.onPostResponse(parser.parseOpenOrders(dataJson), correlationId);
                break;
            case RestEndpointType::OrderStatus:
                listener.onPostResponse(parser.parseOrderStatus(dataJson), correlationId);
                break;
            case RestEndpointType::UserFills:
                listener.onPostResponse(type, parser.parseUserFills(dataJson), correlationId);
                break;
            case RestEndpointType::UserFillsByTime:
                listener.onPostResponse(type, parser.parseUserFillsByTime(dataJson), correlationId);
                break;
            case RestEndpointType::ClearinghouseState:
                listener.onPostResponse(parser.parseClearinghouseState(dataJson), correlationId);
                break;
            case RestEndpointType::VaultDetails:
                listener.onPostResponse(parser.parseVaultDetails(dataJson), correlationId);
                break;
            case RestEndpointType::UserVaultEquities:
                listener.onPostResponse(parser.parseUserVaultEquities(dataJson), correlationId);
                break;
            case RestEndpointType::Portfolio:
                listener.onPostResponse(parser.parsePortfolio(dataJson), correlationId);
                break;
            case RestEndpointType::Referral:
                listener.onPostResponse(parser.parseReferral(dataJson), correlationId);
                break;
            case RestEndpointType::UserRole:
                listener.onPostResponse(parser.parseUserRole(dataJson), correlationId);
                break;
            case RestEndpointType::UserRateLimit:
                listener.onPostResponse(parser.parseUserRateLimit(dataJson), correlationId);
                break;
            case RestEndpointType::PerpDexLimits:
                listener.onPostResponse(parser.parsePerpDexLimits(dataJson), correlationId);
                break;
            case RestEndpointType::PerpDexStatus:
                listener.onPostResponse(parser.parsePerpDexStatus(dataJson), correlationId);
                break;
            case RestEndpointType::PerpDeployAuctionStatus:
                listener.onPostResponse(parser.parsePerpDeployAuctionStatus(dataJson), correlationId);
                break;
            case RestEndpointType::SettledOutcome:
                listener.onPostResponse(parser.parseSettledOutcome(dataJson), correlationId);
                break;
            case RestEndpointType::BorrowLendUserState:
                listener.onPostResponse(parser.parseBorrowLendUserState(dataJson), correlationId);
                break;
            case RestEndpointType::BorrowLendReserveState:
                listener.onPostResponse(parser.parseBorrowLendReserveState(dataJson), correlationId);
                break;
            case RestEndpointType::AllBorrowLendReserveStates:
                listener.onPostResponse(parser.parseAllBorrowLendReserveStates(dataJson), correlationId);
                break;
            case RestEndpointType::SpotDeployState:
                listener.onPostResponse(parser.parseSpotDeployState(dataJson), correlationId);
                break;
            case RestEndpointType::SpotPairDeployAuctionStatus:
                listener.onPostResponse(parser.parseSpotPairDeployAuctionStatus(dataJson), correlationId);
                break;
            case RestEndpointType::UserDexAbstractionState:
                listener.onPostResponse(parser.parseUserDexAbstractionState(dataJson), correlationId);
                break;
            case RestEndpointType::UserAbstraction:
                listener.onPostResponse(parser.parseUserAbstraction(dataJson), correlationId);
                break;
            case RestEndpointType::Delegations:
                listener.onPostResponse(parser.parseDelegations(dataJson), correlationId);
                break;
            case RestEndpointType::DelegatorSummary:
                listener.onPostResponse(parser.parseDelegatorSummary(dataJson), correlationId);
                break;
            case RestEndpointType::DelegatorHistory:
                listener.onPostResponse(parser.parseDelegatorHistory(dataJson), correlationId);
                break;
            case RestEndpointType::DelegatorRewards:
                listener.onPostResponse(parser.parseDelegatorRewards(dataJson), correlationId);
                break;
            case RestEndpointType::ExchangeStatus:
                listener.onPostResponse(parser.parseExchangeStatus(dataJson), correlationId);
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
            listener.onPostResponse(type, parser.parseSimpleResponse(payloadJson), correlationId);
        }
    }

    void handlePostChannelMessage(const std::string& rawMessage,
                                  std::unordered_map<uint64_t, PostRequestInfo>& postRequestInfo,
                                  std::mutex& postRequestInfoMutex,
                                  WebsocketApiListener& listener)
    {
        simdjson::ondemand::parser parser;
        simdjson::padded_string padded(rawMessage.data(), rawMessage.size());
        auto doc = parser.iterate(padded);

        auto data = doc["data"].get_object().value();
        uint64_t id = data["id"].get_uint64().value();
        auto payload = simdjson::to_json_string(data["response"]["payload"]);

        std::optional<PostRequestInfo> info;
        {
            std::lock_guard<std::mutex> lock(postRequestInfoMutex);
            auto it = postRequestInfo.find(id);
            if (it != postRequestInfo.end())
            {
                info = it->second;
                postRequestInfo.erase(it);
            }
        }

        if (!info)
        {
            getLogger()->error("post response with unknown id: {}", id);
            listener.onMessage(rawMessage);
            return;
        }

        std::string payloadStr(payload.value());
        listener.onPostResponse(payloadStr, info->type, info->correlationId);

        if (isAuthenticated(info->type))
            dispatchExchangeActionPostResponse(info->type, payloadStr, info->correlationId, listener);
        else
            dispatchInfoPostResponse(info->type, payloadStr, info->correlationId, listener);
    }
}
