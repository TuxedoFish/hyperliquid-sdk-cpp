#include "hyperliquid/rest/RestApiMessageParser.h"
#include <simdjson.h>
#include <string>
#include "../config/Logger.h"

namespace hyperliquid
{
    struct RestApiMessageParser::Impl
    {
        RestEndpointListener& listener;
        simdjson::ondemand::parser parser;
        simdjson::padded_string padded;

        explicit Impl(RestEndpointListener& listener) : listener(listener) {}

        void parse(const std::string& message, RestEndpointType type,
                   std::optional<uint64_t> correlationId = std::nullopt)
        {
            switch (type)
            {
            case RestEndpointType::SpotMeta:
                listener.onSpotMeta(parseSpotMeta(message), correlationId);
                break;
            case RestEndpointType::Meta:
                listener.onMeta(parseMeta(message), correlationId);
                break;
            case RestEndpointType::OutcomeMeta:
                listener.onOutcomeMeta(parseOutcomeMeta(message), correlationId);
                break;
            case RestEndpointType::SettledOutcome:
                listener.onSettledOutcome(parseSettledOutcome(message), correlationId);
                break;
            case RestEndpointType::PerpDexs:
                listener.onPerpDexs(parsePerpDexs(message), correlationId);
                break;
            case RestEndpointType::PerpsAtOpenInterestCap:
                listener.onPerpsAtOpenInterestCap(parsePerpsAtOpenInterestCap(message), correlationId);
                break;
            case RestEndpointType::PredictedFundings:
                listener.onPredictedFundings(parsePredictedFundings(message), correlationId);
                break;
            case RestEndpointType::PerpAnnotation:
                listener.onPerpAnnotation(parsePerpAnnotation(message), correlationId);
                break;
            case RestEndpointType::PerpCategories:
                listener.onPerpCategories(parsePerpCategories(message), correlationId);
                break;
            case RestEndpointType::PerpConciseAnnotations:
                listener.onPerpConciseAnnotations(parsePerpConciseAnnotations(message), correlationId);
                break;
            case RestEndpointType::AllPerpMetas:
                listener.onAllPerpMetas(parseAllPerpMetas(message), correlationId);
                break;
            case RestEndpointType::PerpDexLimits:
                listener.onPerpDexLimits(parsePerpDexLimits(message), correlationId);
                break;
            case RestEndpointType::PerpDexStatus:
                listener.onPerpDexStatus(parsePerpDexStatus(message), correlationId);
                break;
            case RestEndpointType::PerpDeployAuctionStatus:
                listener.onPerpDeployAuctionStatus(parsePerpDeployAuctionStatus(message), correlationId);
                break;
            case RestEndpointType::L2Book:
                listener.onL2Book(parseL2Book(message), correlationId);
                break;
            case RestEndpointType::CandleSnapshot:
                listener.onCandleSnapshot(parseCandleSnapshot(message), correlationId);
                break;
            case RestEndpointType::AllMids:
                listener.onAllMids(parseAllMids(message), correlationId);
                break;
            case RestEndpointType::OpenOrders:
                listener.onOpenOrders(parseOpenOrders(message), correlationId);
                break;
            case RestEndpointType::OrderStatus:
                listener.onOrderStatus(parseOrderStatus(message), correlationId);
                break;
            case RestEndpointType::UserFills:
                listener.onUserFills(parseUserFills(message), correlationId);
                break;
            case RestEndpointType::UserFillsByTime:
                listener.onUserFillsByTime(parseUserFillsByTime(message), correlationId);
                break;
            case RestEndpointType::ClearinghouseState:
                listener.onClearinghouseState(parseClearinghouseState(message), correlationId);
                break;
            case RestEndpointType::UserRateLimit:
                listener.onUserRateLimit(parseUserRateLimit(message), correlationId);
                break;
            case RestEndpointType::MetaAndAssetCtxs:
                listener.onMetaAndAssetCtxs(parseMetaAndAssetCtxs(message), correlationId);
                break;
            case RestEndpointType::SpotMetaAndAssetCtxs:
                listener.onSpotMetaAndAssetCtxs(parseSpotMetaAndAssetCtxs(message), correlationId);
                break;
            case RestEndpointType::SpotClearinghouseState:
                listener.onSpotClearinghouseState(parseSpotClearinghouseState(message), correlationId);
                break;
            case RestEndpointType::SpotDeployState:
                listener.onSpotDeployState(parseSpotDeployState(message), correlationId);
                break;
            case RestEndpointType::SpotPairDeployAuctionStatus:
                listener.onSpotPairDeployAuctionStatus(parseSpotPairDeployAuctionStatus(message), correlationId);
                break;
            case RestEndpointType::FrontendOpenOrders:
                listener.onFrontendOpenOrders(parseFrontendOpenOrders(message), correlationId);
                break;
            case RestEndpointType::HistoricalOrders:
                listener.onHistoricalOrders(parseHistoricalOrders(message), correlationId);
                break;
            case RestEndpointType::UserTwapSliceFills:
                listener.onUserTwapSliceFills(parseUserTwapSliceFills(message), correlationId);
                break;
            case RestEndpointType::SubAccounts:
                listener.onSubAccounts(parseSubAccounts(message), correlationId);
                break;
            case RestEndpointType::UserFees:
                listener.onUserFees(parseUserFees(message), correlationId);
                break;
            case RestEndpointType::MaxBuilderFee:
                listener.onMaxBuilderFee(parseMaxBuilderFee(message), correlationId);
                break;
            case RestEndpointType::ApprovedBuilders:
                listener.onApprovedBuilders(parseApprovedBuilders(message), correlationId);
                break;
            case RestEndpointType::VaultDetails:
                listener.onVaultDetails(parseVaultDetails(message), correlationId);
                break;
            case RestEndpointType::UserVaultEquities:
                listener.onUserVaultEquities(parseUserVaultEquities(message), correlationId);
                break;
            case RestEndpointType::Portfolio:
                listener.onPortfolio(parsePortfolio(message), correlationId);
                break;
            case RestEndpointType::Referral:
                listener.onReferral(parseReferral(message), correlationId);
                break;
            case RestEndpointType::UserRole:
                listener.onUserRole(parseUserRole(message), correlationId);
                break;
            case RestEndpointType::BorrowLendUserState:
                listener.onBorrowLendUserState(parseBorrowLendUserState(message), correlationId);
                break;
            case RestEndpointType::BorrowLendReserveState:
                listener.onBorrowLendReserveState(parseBorrowLendReserveState(message), correlationId);
                break;
            case RestEndpointType::AllBorrowLendReserveStates:
                listener.onAllBorrowLendReserveStates(parseAllBorrowLendReserveStates(message), correlationId);
                break;
            case RestEndpointType::PlaceOrder:
                listener.onPlaceOrder(parsePlaceOrder(message), correlationId);
                break;
            case RestEndpointType::CancelOrder:
            case RestEndpointType::CancelOrderByCloid:
                listener.onCancelOrder(parseCancelOrder(message), correlationId);
                break;
            case RestEndpointType::ModifyOrder:
            case RestEndpointType::BatchModifyOrder:
                listener.onModifyOrder(parseModifyOrder(message), correlationId);
                break;
            case RestEndpointType::UpdateLeverage:
            case RestEndpointType::UpdateIsolatedMargin:
            case RestEndpointType::ScheduleCancel:
            case RestEndpointType::ApproveAgent:
            case RestEndpointType::AgentSetAbstraction:
            case RestEndpointType::VaultTransfer:
            case RestEndpointType::Hip3LiquidatorTransfer:
            case RestEndpointType::UsdClassTransfer:
            case RestEndpointType::SendAsset:
            case RestEndpointType::UsdSend:
            case RestEndpointType::SpotSend:
            case RestEndpointType::Withdraw3:
            case RestEndpointType::ApproveBuilderFee:
            case RestEndpointType::UserSetAbstraction:
            case RestEndpointType::CDeposit:
            case RestEndpointType::CWithdraw:
            case RestEndpointType::TokenDelegate:
            case RestEndpointType::SendToEvmWithData:
            case RestEndpointType::UserDexAbstraction:
            case RestEndpointType::AgentSendAsset:
            case RestEndpointType::ReserveRequestWeight:
            case RestEndpointType::Noop:
                listener.onSimpleResponse(parseSimpleResponse(message), correlationId);
                break;
            case RestEndpointType::TwapOrder:
                listener.onTwapOrder(parseTwapOrder(message), correlationId);
                break;
            case RestEndpointType::TwapCancel:
                listener.onTwapCancel(parseTwapCancel(message), correlationId);
                break;
            case RestEndpointType::Delegations:
                listener.onDelegations(parseDelegations(message), correlationId);
                break;
            case RestEndpointType::DelegatorSummary:
                listener.onDelegatorSummary(parseDelegatorSummary(message), correlationId);
                break;
            case RestEndpointType::DelegatorHistory:
                listener.onDelegatorHistory(parseDelegatorHistory(message), correlationId);
                break;
            case RestEndpointType::DelegatorRewards:
                listener.onDelegatorRewards(parseDelegatorRewards(message), correlationId);
                break;
            default:
                getLogger()->error("RestMessageParser: unhandled RestEndpointType: {}", toString(type));
                break;
            }
        }

        PlaceOrderResponse parsePlaceOrder(const std::string& message)
        {
            PlaceOrderResponse response;
            padded = simdjson::padded_string(message.data(), message.size());
            auto doc = parser.iterate(padded);

            try
            {
                response.status = std::string(doc["status"].get_string().value());

                if (response.status != "ok") return response;

                auto resp = doc["response"].get_object().value();
                response.type = std::string(resp["type"].get_string().value());

                if (response.type == "default") return response;

                auto statuses = resp["data"]["statuses"].get_array().value();
                for (auto entry : statuses)
                {
                    auto obj = entry.get_object().value();
                    OrderStatusResult result;

                    simdjson::ondemand::value resting;
                    if (obj["resting"].get(resting) == simdjson::SUCCESS)
                    {
                        auto restingObj = resting.get_object().value();
                        OrderStatusResting rest;
                        rest.oid = restingObj["oid"].get_uint64().value();
                        simdjson::ondemand::value restingCloid;
                        if (restingObj["cloid"].get(restingCloid) == simdjson::SUCCESS && !restingCloid.is_null())
                            rest.cloid = std::string(restingCloid.get_string().value());
                        result.resting = std::move(rest);
                    }

                    simdjson::ondemand::value filled;
                    if (obj["filled"].get(filled) == simdjson::SUCCESS)
                    {
                        auto filledObj = filled.get_object().value();
                        OrderStatusFilled fill;
                        fill.totalSz = std::string(filledObj["totalSz"].get_string().value());
                        fill.avgPx = std::string(filledObj["avgPx"].get_string().value());
                        fill.oid = filledObj["oid"].get_uint64().value();
                        simdjson::ondemand::value filledCloid;
                        if (filledObj["cloid"].get(filledCloid) == simdjson::SUCCESS && !filledCloid.is_null())
                            fill.cloid = std::string(filledCloid.get_string().value());
                        result.filled = std::move(fill);
                    }

                    simdjson::ondemand::value error;
                    if (obj["error"].get(error) == simdjson::SUCCESS)
                    {
                        result.error = std::string(error.get_string().value());
                    }

                    response.statuses.push_back(std::move(result));
                }
            }
            catch (const simdjson::simdjson_error& err)
            {
                getLogger()->error("RestMessageParser: parse error in placeOrder: {}\n  raw: {}", err.what(), message);
            }

            return response;
        }

        CancelOrderResponse parseCancelOrder(const std::string& message)
        {
            CancelOrderResponse response;
            padded = simdjson::padded_string(message.data(), message.size());
            auto doc = parser.iterate(padded);

            try
            {
                response.status = std::string(doc["status"].get_string().value());

                if (response.status != "ok") return response;

                auto resp = doc["response"].get_object().value();
                response.type = std::string(resp["type"].get_string().value());

                if (response.type == "default") return response;

                auto statuses = resp["data"]["statuses"].get_array().value();
                for (auto entry : statuses)
                {
                    CancelStatusResult result;

                    // statuses can be either a string "success" or an object with "error"
                    simdjson::ondemand::json_type entryType = entry.type().value();
                    if (entryType == simdjson::ondemand::json_type::string)
                    {
                        result.success = std::string(entry.get_string().value());
                    }
                    else if (entryType == simdjson::ondemand::json_type::object)
                    {
                        auto obj = entry.get_object().value();
                        simdjson::ondemand::value error;
                        if (obj["error"].get(error) == simdjson::SUCCESS)
                        {
                            result.error = std::string(error.get_string().value());
                        }
                    }

                    response.statuses.push_back(std::move(result));
                }
            }
            catch (const simdjson::simdjson_error& err)
            {
                getLogger()->error("RestMessageParser: parse error in cancelOrder: {}\n  raw: {}", err.what(), message);
            }

            return response;
        }

        ModifyOrderResponse parseModifyOrder(const std::string& message)
        {
            ModifyOrderResponse response;
            padded = simdjson::padded_string(message.data(), message.size());
            auto doc = parser.iterate(padded);

            try
            {
                response.status = std::string(doc["status"].get_string().value());

                if (response.status != "ok") return response;

                auto resp = doc["response"].get_object().value();
                response.type = std::string(resp["type"].get_string().value());

                if (response.type == "default") return response;

                auto statuses = resp["data"]["statuses"].get_array().value();
                for (auto entry : statuses)
                {
                    auto obj = entry.get_object().value();
                    OrderStatusResult result;

                    simdjson::ondemand::value resting;
                    if (obj["resting"].get(resting) == simdjson::SUCCESS)
                    {
                        auto restingObj = resting.get_object().value();
                        OrderStatusResting rest;
                        rest.oid = restingObj["oid"].get_uint64().value();
                        simdjson::ondemand::value restingCloid;
                        if (restingObj["cloid"].get(restingCloid) == simdjson::SUCCESS && !restingCloid.is_null())
                            rest.cloid = std::string(restingCloid.get_string().value());
                        result.resting = std::move(rest);
                    }

                    simdjson::ondemand::value filled;
                    if (obj["filled"].get(filled) == simdjson::SUCCESS)
                    {
                        auto filledObj = filled.get_object().value();
                        OrderStatusFilled fill;
                        fill.totalSz = std::string(filledObj["totalSz"].get_string().value());
                        fill.avgPx = std::string(filledObj["avgPx"].get_string().value());
                        fill.oid = filledObj["oid"].get_uint64().value();
                        simdjson::ondemand::value filledCloid;
                        if (filledObj["cloid"].get(filledCloid) == simdjson::SUCCESS && !filledCloid.is_null())
                            fill.cloid = std::string(filledCloid.get_string().value());
                        result.filled = std::move(fill);
                    }

                    simdjson::ondemand::value error;
                    if (obj["error"].get(error) == simdjson::SUCCESS)
                    {
                        result.error = std::string(error.get_string().value());
                    }

                    response.statuses.push_back(std::move(result));
                }
            }
            catch (const simdjson::simdjson_error& err)
            {
                getLogger()->error("RestMessageParser: parse error in modifyOrder: {}\n  raw: {}", err.what(), message);
            }

            return response;
        }

        SimpleResponse parseSimpleResponse(const std::string& message)
        {
            SimpleResponse response;
            padded = simdjson::padded_string(message.data(), message.size());
            auto doc = parser.iterate(padded);

            try
            {
                response.status = std::string(doc["status"].get_string().value());

                if (response.status != "ok")
                {
                    simdjson::ondemand::value resp;
                    if (doc["response"].get(resp) == simdjson::SUCCESS
                        && resp.type().value() == simdjson::ondemand::json_type::string)
                    {
                        response.error = std::string(resp.get_string().value());
                    }
                    return response;
                }

                auto resp = doc["response"].get_object().value();
                response.type = std::string(resp["type"].get_string().value());
            }
            catch (const simdjson::simdjson_error& err)
            {
                getLogger()->error("RestMessageParser: parse error in simpleResponse: {}\n  raw: {}", err.what(), message);
                if (response.status.empty())
                {
                    response.status = "err";
                    response.error = message;
                }
            }

            return response;
        }

        TwapOrderResponse parseTwapOrder(const std::string& message)
        {
            TwapOrderResponse response;
            padded = simdjson::padded_string(message.data(), message.size());
            auto doc = parser.iterate(padded);

            try
            {
                response.status = std::string(doc["status"].get_string().value());

                if (response.status != "ok")
                {
                    simdjson::ondemand::value resp;
                    if (doc["response"].get(resp) == simdjson::SUCCESS
                        && resp.type().value() == simdjson::ondemand::json_type::string)
                    {
                        response.error = std::string(resp.get_string().value());
                    }
                    return response;
                }

                auto resp = doc["response"].get_object().value();
                response.type = std::string(resp["type"].get_string().value());

                simdjson::ondemand::value statusVal;
                if (resp["data"]["status"].get(statusVal) == simdjson::SUCCESS)
                {
                    auto statusObj = statusVal.get_object().value();

                    simdjson::ondemand::value running;
                    if (statusObj["running"].get(running) == simdjson::SUCCESS)
                    {
                        auto runningObj = running.get_object().value();
                        response.twapId = runningObj["twapId"].get_uint64().value();
                    }

                    simdjson::ondemand::value error;
                    if (statusObj["error"].get(error) == simdjson::SUCCESS)
                        response.error = std::string(error.get_string().value());
                }
            }
            catch (const simdjson::simdjson_error& err)
            {
                getLogger()->error("RestMessageParser: parse error in twapOrder: {}\n  raw: {}", err.what(), message);
            }

            return response;
        }

        TwapCancelResponse parseTwapCancel(const std::string& message)
        {
            TwapCancelResponse response;
            padded = simdjson::padded_string(message.data(), message.size());
            auto doc = parser.iterate(padded);

            try
            {
                response.status = std::string(doc["status"].get_string().value());

                if (response.status != "ok")
                {
                    simdjson::ondemand::value resp;
                    if (doc["response"].get(resp) == simdjson::SUCCESS
                        && resp.type().value() == simdjson::ondemand::json_type::string)
                    {
                        response.error = std::string(resp.get_string().value());
                    }
                    return response;
                }

                auto resp = doc["response"].get_object().value();
                response.type = std::string(resp["type"].get_string().value());

                simdjson::ondemand::value statusVal;
                if (resp["data"]["status"].get(statusVal) == simdjson::SUCCESS)
                {
                    auto statusType = statusVal.type().value();
                    if (statusType == simdjson::ondemand::json_type::string)
                    {
                        response.success = std::string(statusVal.get_string().value());
                    }
                    else if (statusType == simdjson::ondemand::json_type::object)
                    {
                        auto statusObj = statusVal.get_object().value();
                        simdjson::ondemand::value error;
                        if (statusObj["error"].get(error) == simdjson::SUCCESS)
                            response.error = std::string(error.get_string().value());
                    }
                }
            }
            catch (const simdjson::simdjson_error& err)
            {
                getLogger()->error("RestMessageParser: parse error in twapCancel: {}\n  raw: {}", err.what(), message);
            }

            return response;
        }

        static void parseOutcomeSpec(simdjson::ondemand::object& obj, Outcome& outcome)
        {
            outcome.outcome = static_cast<int>(obj["outcome"].get_int64().value());
            outcome.name = std::string(obj["name"].get_string().value());
            outcome.descriptionRaw = std::string(obj["description"].get_string().value());
            outcome.description = parseOutcomeDescription(outcome.descriptionRaw);

            auto sideSpecs = obj["sideSpecs"].get_array().value();
            for (auto sideEntry : sideSpecs)
            {
                auto sideObj = sideEntry.get_object().value();
                OutcomeSideSpec spec;
                spec.name = std::string(sideObj["name"].get_string().value());
                int64_t token;
                if (!sideObj["token"].get_int64().get(token))
                    spec.token = static_cast<int>(token);
                outcome.sideSpecs.push_back(std::move(spec));
            }

            std::string_view quoteToken;
            if (!obj["quoteToken"].get_string().get(quoteToken))
                outcome.quoteToken = std::string(quoteToken);

            std::string_view deployer;
            if (!obj["deployer"].get_string().get(deployer))
                outcome.deployer = std::string(deployer);
        }

        OutcomeMetaResponse parseOutcomeMeta(const std::string& message)
        {
            OutcomeMetaResponse response;
            padded = simdjson::padded_string(message.data(), message.size());
            auto doc = parser.iterate(padded);

            try
            {
                auto outcomes = doc["outcomes"].get_array().value();
                for (auto entry : outcomes)
                {
                    auto obj = entry.get_object().value();
                    Outcome outcome;
                    parseOutcomeSpec(obj, outcome);
                    response.outcomes.push_back(std::move(outcome));
                }
            }
            catch (const simdjson::simdjson_error& e)
            {
                getLogger()->error("RestMessageParser: parse error in outcomeMeta: {}\n  raw: {}", e.what(), message);
            }

            return response;
        }

        SettledOutcomeResponse parseSettledOutcome(const std::string& message)
        {
            SettledOutcomeResponse response;
            padded = simdjson::padded_string(message.data(), message.size());
            auto doc = parser.iterate(padded);

            try
            {
                if (doc.type().value() == simdjson::ondemand::json_type::null)
                    return response;

                auto root = doc.get_object().value();
                response.isSettled = true;

                auto spec = root["spec"].get_object().value();
                parseOutcomeSpec(spec, response.spec);

                response.settleFraction = toDouble(root["settleFraction"].get_string().value());
                response.details = std::string(root["details"].get_string().value());

                simdjson::ondemand::object questionObj;
                if (!root["question"].get_object().get(questionObj))
                {
                    SettledOutcomeQuestion question;
                    auto questionId = questionObj["question"].get_object().value();

                    int64_t active;
                    if (!questionId["active"].get_int64().get(active))
                    {
                        question.isSettled = false;
                        question.questionId = static_cast<int>(active);
                    }
                    else
                    {
                        question.isSettled = true;
                        question.questionId = static_cast<int>(questionId["settled"].get_int64().value());
                    }

                    question.name = std::string(questionObj["name"].get_string().value());
                    question.description = std::string(questionObj["description"].get_string().value());
                    response.question = std::move(question);
                }
            }
            catch (const simdjson::simdjson_error& e)
            {
                getLogger()->error("RestMessageParser: parse error in settledOutcome: {}\n  raw: {}", e.what(), message);
            }

            return response;
        }

        PerpDexsResponse parsePerpDexs(const std::string& message)
        {
            PerpDexsResponse response;
            padded = simdjson::padded_string(message.data(), message.size());
            auto doc = parser.iterate(padded);

            try
            {
                auto arr = doc.get_array().value();
                bool firstElement = true;
                for (auto entry : arr)
                {
                    if (firstElement) {
                        firstElement = false; // Always null
                        continue;
                    }
                    auto obj = entry.get_object().value();
                    PerpDex dex;
                    dex.name = std::string(obj["name"].get_string().value());
                    dex.fullName = std::string(obj["fullName"].get_string().value());
                    dex.deployer = std::string(obj["deployer"].get_string().value());
                    simdjson::ondemand::value oracleUpdater;
                    if (obj["oracleUpdater"].get(oracleUpdater) == simdjson::SUCCESS && !oracleUpdater.is_null())
                        dex.oracleUpdater = std::string(oracleUpdater.get_string().value());

                    simdjson::ondemand::value feeRecipient;
                    if (obj["feeRecipient"].get(feeRecipient) == simdjson::SUCCESS && !feeRecipient.is_null())
                        dex.feeRecipient = std::string(feeRecipient.get_string().value());

                    auto oiCaps = obj["assetToStreamingOiCap"].get_array().value();
                    for (auto pair : oiCaps)
                    {
                        auto pairArr = pair.get_array().value();
                        auto iter = pairArr.begin();
                        if (iter == pairArr.end()) continue;
                        std::string asset = std::string((*iter).get_string().value());
                        ++iter;
                        if (iter == pairArr.end()) continue;
                        std::string cap = std::string((*iter).get_string().value());
                        dex.assetToStreamingOiCap.emplace_back(asset, cap);
                    }

                    auto fundingMults = obj["assetToFundingMultiplier"].get_array().value();
                    for (auto pair : fundingMults)
                    {
                        auto pairArr = pair.get_array().value();
                        auto iter = pairArr.begin();
                        if (iter == pairArr.end()) continue;
                        std::string asset = std::string((*iter).get_string().value());
                        ++iter;
                        if (iter == pairArr.end()) continue;
                        std::string multiplier = std::string((*iter).get_string().value());
                        dex.assetToFundingMultiplier.emplace_back(asset, multiplier);
                    }

                    response.dexes.push_back(std::move(dex));
                }
            }
            catch (const simdjson::simdjson_error& err)
            {
                getLogger()->error("RestMessageParser: parse error in perpDexs: {}\n  raw: {}", err.what(), message);
            }

            return response;
        }

        DelegationsResponse parseDelegations(const std::string& message)
        {
            DelegationsResponse response;
            padded = simdjson::padded_string(message.data(), message.size());
            auto doc = parser.iterate(padded);

            try
            {
                auto arr = doc.get_array().value();
                for (auto entry : arr)
                {
                    auto obj = entry.get_object().value();
                    Delegation delegation;
                    delegation.validator = std::string(obj["validator"].get_string().value());
                    delegation.amount = parseNumberField(obj, "amount");
                    delegation.lockedUntilTimestamp = obj["lockedUntilTimestamp"].get_uint64().value();
                    response.delegations.push_back(std::move(delegation));
                }
            }
            catch (const simdjson::simdjson_error& err)
            {
                getLogger()->error("RestMessageParser: parse error in delegations: {}\n  raw: {}", err.what(), message);
            }

            return response;
        }

        DelegatorSummaryResponse parseDelegatorSummary(const std::string& message)
        {
            DelegatorSummaryResponse response{};
            padded = simdjson::padded_string(message.data(), message.size());
            auto doc = parser.iterate(padded);

            try
            {
                auto obj = doc.get_object().value();
                response.delegated = parseNumberField(obj, "delegated");
                response.undelegated = parseNumberField(obj, "undelegated");
                response.totalPendingWithdrawal = parseNumberField(obj, "totalPendingWithdrawal");
                response.nPendingWithdrawals = static_cast<int>(obj["nPendingWithdrawals"].get_int64().value());
            }
            catch (const simdjson::simdjson_error& err)
            {
                getLogger()->error("RestMessageParser: parse error in delegatorSummary: {}\n  raw: {}", err.what(), message);
            }

            return response;
        }

        PerpDexLimitsResponse parsePerpDexLimits(const std::string& message)
        {
            PerpDexLimitsResponse response;
            padded = simdjson::padded_string(message.data(), message.size());
            auto doc = parser.iterate(padded);

            try
            {
                if (doc.type().value() == simdjson::ondemand::json_type::null)
                    return response;

                auto obj = doc.get_object().value();
                response.exists = true;
                response.totalOiCap = parseNumberField(obj, "totalOiCap");
                response.oiSzCapPerPerp = parseNumberField(obj, "oiSzCapPerPerp");
                response.maxTransferNtl = parseNumberField(obj, "maxTransferNtl");

                auto coinToOiCap = obj["coinToOiCap"].get_array().value();
                for (auto entry : coinToOiCap)
                {
                    auto pair = entry.get_array().value();
                    auto iter = pair.begin();
                    if (iter == pair.end()) continue;

                    PerpDexLimitsCoinCap cap;
                    cap.coin = std::string((*iter).get_string().value());
                    ++iter;
                    if (iter == pair.end()) continue;
                    cap.oiCap = toDouble((*iter).get_string().value());
                    response.coinToOiCap.push_back(std::move(cap));
                }
            }
            catch (const simdjson::simdjson_error& err)
            {
                getLogger()->error("RestMessageParser: parse error in perpDexLimits: {}\n  raw: {}", err.what(), message);
            }

            return response;
        }

        DelegatorHistoryResponse parseDelegatorHistory(const std::string& message)
        {
            DelegatorHistoryResponse response;
            padded = simdjson::padded_string(message.data(), message.size());
            auto doc = parser.iterate(padded);

            try
            {
                auto arr = doc.get_array().value();
                for (auto entry : arr)
                {
                    auto obj = entry.get_object().value();
                    DelegatorHistoryEntry historyEntry{};
                    historyEntry.time = obj["time"].get_uint64().value();
                    historyEntry.hash = std::string(obj["hash"].get_string().value());

                    auto deltaObj = obj["delta"].get_object().value();

                    simdjson::ondemand::value delegateVal;
                    simdjson::ondemand::value cDepositVal;
                    simdjson::ondemand::value withdrawalVal;
                    if (deltaObj["delegate"].get(delegateVal) == simdjson::SUCCESS)
                    {
                        auto d = delegateVal.get_object().value();
                        historyEntry.delta.type = DelegatorHistoryDeltaType::Delegate;
                        historyEntry.delta.validator = std::string(d["validator"].get_string().value());
                        historyEntry.delta.amount = parseNumberField(d, "amount");
                        historyEntry.delta.isUndelegate = d["isUndelegate"].get_bool().value();
                    }
                    else if (deltaObj["cDeposit"].get(cDepositVal) == simdjson::SUCCESS)
                    {
                        auto d = cDepositVal.get_object().value();
                        historyEntry.delta.type = DelegatorHistoryDeltaType::CDeposit;
                        historyEntry.delta.amount = parseNumberField(d, "amount");
                    }
                    else if (deltaObj["withdrawal"].get(withdrawalVal) == simdjson::SUCCESS)
                    {
                        auto d = withdrawalVal.get_object().value();
                        historyEntry.delta.type = DelegatorHistoryDeltaType::Withdrawal;
                        historyEntry.delta.amount = parseNumberField(d, "amount");
                        historyEntry.delta.phase = stringToWithdrawalPhase(d["phase"].get_string().value());
                    }
                    else
                    {
                        historyEntry.delta.type = DelegatorHistoryDeltaType::Unknown;
                    }

                    response.history.push_back(std::move(historyEntry));
                }
            }
            catch (const simdjson::simdjson_error& err)
            {
                getLogger()->error("RestMessageParser: parse error in delegatorHistory: {}\n  raw: {}", err.what(), message);
            }

            return response;
        }

        PerpDexStatusResponse parsePerpDexStatus(const std::string& message)
        {
            PerpDexStatusResponse response;
            padded = simdjson::padded_string(message.data(), message.size());
            auto doc = parser.iterate(padded);

            try
            {
                if (doc.type().value() == simdjson::ondemand::json_type::null)
                    return response;

                auto obj = doc.get_object().value();
                response.exists = true;
                response.totalNetDeposit = parseNumberField(obj, "totalNetDeposit");
            }
            catch (const simdjson::simdjson_error& err)
            {
                getLogger()->error("RestMessageParser: parse error in perpDexStatus: {}\n  raw: {}", err.what(), message);
            }

            return response;
        }

        DelegatorRewardsResponse parseDelegatorRewards(const std::string& message)
        {
            DelegatorRewardsResponse response;
            padded = simdjson::padded_string(message.data(), message.size());
            auto doc = parser.iterate(padded);

            try
            {
                auto arr = doc.get_array().value();
                for (auto entry : arr)
                {
                    auto obj = entry.get_object().value();
                    DelegatorReward reward{};
                    reward.time = obj["time"].get_uint64().value();
                    reward.source = stringToDelegatorRewardSource(obj["source"].get_string().value());
                    reward.totalAmount = parseNumberField(obj, "totalAmount");
                    response.rewards.push_back(std::move(reward));
                }
            }
            catch (const simdjson::simdjson_error& err)
            {
                getLogger()->error("RestMessageParser: parse error in delegatorRewards: {}\n  raw: {}", err.what(), message);
            }

            return response;
        }

        PerpsAtOpenInterestCapResponse parsePerpsAtOpenInterestCap(const std::string& message)
        {
            PerpsAtOpenInterestCapResponse response;
            padded = simdjson::padded_string(message.data(), message.size());
            auto doc = parser.iterate(padded);

            try
            {
                auto arr = doc.get_array().value();
                for (auto entry : arr)
                {
                    response.coins.emplace_back(entry.get_string().value());
                }
            }
            catch (const simdjson::simdjson_error& e)
            {
                getLogger()->error("RestMessageParser: parse error in perpsAtOpenInterestCap: {}\n  raw: {}", e.what(), message);
            }

            return response;
        }

        PredictedFundingsResponse parsePredictedFundings(const std::string& message)
        {
            PredictedFundingsResponse response;
            padded = simdjson::padded_string(message.data(), message.size());
            auto doc = parser.iterate(padded);

            try
            {
                auto arr = doc.get_array().value();
                for (auto entry : arr)
                {
                    auto pair = entry.get_array().value();
                    auto iter = pair.begin();
                    if (iter == pair.end()) continue;

                    PredictedFundingEntry pf;
                    pf.coin = std::string((*iter).get_string().value());
                    ++iter;
                    if (iter == pair.end()) continue;

                    auto venues = (*iter).get_array().value();
                    for (auto venueEntry : venues)
                    {
                        auto venuePair = venueEntry.get_array().value();
                        auto vIter = venuePair.begin();
                        if (vIter == venuePair.end()) continue;

                        PredictedFundingVenue venue;
                        venue.venue = std::string((*vIter).get_string().value());
                        ++vIter;
                        if (vIter == venuePair.end())
                        {
                            pf.venues.push_back(std::move(venue));
                            continue;
                        }

                        // The exchange returns null here (rather than an object) when it has no
                        // funding data for this coin on this venue - leave fundingRate/
                        // nextFundingTime unset in that case rather than treating it as an error.
                        simdjson::ondemand::value objVal;
                        if ((*vIter).get(objVal) == simdjson::SUCCESS && !objVal.is_null())
                        {
                            auto obj = objVal.get_object().value();

                            simdjson::ondemand::value rateVal;
                            if (obj["fundingRate"].get(rateVal) == simdjson::SUCCESS && !rateVal.is_null())
                            {
                                std::string_view sv;
                                venue.fundingRate = !rateVal.get_string().get(sv) ? toDouble(sv) : rateVal.get_double().value();
                            }

                            simdjson::ondemand::value timeVal;
                            if (obj["nextFundingTime"].get(timeVal) == simdjson::SUCCESS && !timeVal.is_null())
                                venue.nextFundingTime = timeVal.get_uint64().value();
                        }

                        pf.venues.push_back(std::move(venue));
                    }

                    response.fundings.push_back(std::move(pf));
                }
            }
            catch (const simdjson::simdjson_error& e)
            {
                getLogger()->error("RestMessageParser: parse error in predictedFundings: {}\n  raw: {}", e.what(), message);
            }

            return response;
        }

        PerpAnnotationResponse parsePerpAnnotation(const std::string& message)
        {
            PerpAnnotationResponse response;
            padded = simdjson::padded_string(message.data(), message.size());
            auto doc = parser.iterate(padded);

            try
            {
                // The exchange returns a bare `null` (rather than an object) for a coin with no
                // annotation data - leave category/description empty in that case, it's not a
                // parse error.
                if (doc.type().value() != simdjson::ondemand::json_type::null)
                {
                    auto obj = doc.get_object().value();
                    response.category = std::string(obj["category"].get_string().value());
                    response.description = std::string(obj["description"].get_string().value());
                }
            }
            catch (const simdjson::simdjson_error& e)
            {
                getLogger()->error("RestMessageParser: parse error in perpAnnotation: {}\n  raw: {}", e.what(), message);
            }

            return response;
        }

        PerpCategoriesResponse parsePerpCategories(const std::string& message)
        {
            PerpCategoriesResponse response;
            padded = simdjson::padded_string(message.data(), message.size());
            auto doc = parser.iterate(padded);

            try
            {
                auto arr = doc.get_array().value();
                for (auto entry : arr)
                {
                    auto pair = entry.get_array().value();
                    auto iter = pair.begin();
                    if (iter == pair.end()) continue;

                    PerpCategoryEntry pc;
                    pc.coin = std::string((*iter).get_string().value());
                    ++iter;
                    if (iter == pair.end()) continue;
                    pc.category = std::string((*iter).get_string().value());

                    response.categories.push_back(std::move(pc));
                }
            }
            catch (const simdjson::simdjson_error& e)
            {
                getLogger()->error("RestMessageParser: parse error in perpCategories: {}\n  raw: {}", e.what(), message);
            }

            return response;
        }

        PerpConciseAnnotationsResponse parsePerpConciseAnnotations(const std::string& message)
        {
            PerpConciseAnnotationsResponse response;
            padded = simdjson::padded_string(message.data(), message.size());
            auto doc = parser.iterate(padded);

            try
            {
                auto arr = doc.get_array().value();
                for (auto entry : arr)
                {
                    auto pair = entry.get_array().value();
                    auto iter = pair.begin();
                    if (iter == pair.end()) continue;

                    PerpConciseAnnotationEntry pc;
                    pc.coin = std::string((*iter).get_string().value());
                    ++iter;
                    if (iter == pair.end()) continue;

                    auto obj = (*iter).get_object().value();
                    pc.category = std::string(obj["category"].get_string().value());

                    simdjson::ondemand::array keywords;
                    if (!obj["keywords"].get_array().get(keywords))
                    {
                        for (auto kw : keywords)
                            pc.keywords.emplace_back(kw.get_string().value());
                    }

                    response.annotations.push_back(std::move(pc));
                }
            }
            catch (const simdjson::simdjson_error& e)
            {
                getLogger()->error("RestMessageParser: parse error in perpConciseAnnotations: {}\n  raw: {}", e.what(), message);
            }

            return response;
        }

        AllPerpMetasResponse parseAllPerpMetas(const std::string& message)
        {
            AllPerpMetasResponse response;
            padded = simdjson::padded_string(message.data(), message.size());
            auto doc = parser.iterate(padded);

            try
            {
                // Each top-level entry is a flat per-dex meta object ({universe, marginTables,
                // collateralToken}) - unlike metaAndAssetCtxs, this endpoint does not pair each
                // dex with live asset-context data (confirmed against real testnet responses).
                auto arr = doc.get_array().value();
                for (auto entry : arr)
                {
                    auto metaObj = entry.get_object().value();

                    PerpDexMeta dexMeta;

                    auto universe = metaObj["universe"].get_array().value();
                    for (auto u : universe)
                    {
                        auto uObj = u.get_object().value();
                        AssetMeta asset;
                        asset.name = std::string(uObj["name"].get_string().value());
                        asset.szDecimals = static_cast<int>(uObj["szDecimals"].get_int64().value());
                        asset.maxLeverage = static_cast<int>(uObj["maxLeverage"].get_int64().value());
                        dexMeta.universe.push_back(std::move(asset));
                    }

                    simdjson::ondemand::array marginTables;
                    if (!metaObj["marginTables"].get_array().get(marginTables))
                    {
                        for (auto mt : marginTables)
                        {
                            auto mtPair = mt.get_array().value();
                            auto mtIter = mtPair.begin();
                            if (mtIter == mtPair.end()) continue;

                            MarginTableEntry table;
                            table.id = static_cast<int>((*mtIter).get_int64().value());
                            ++mtIter;
                            if (mtIter == mtPair.end()) continue;

                            auto tableObj = (*mtIter).get_object().value();
                            table.description = std::string(tableObj["description"].get_string().value());

                            auto tiers = tableObj["marginTiers"].get_array().value();
                            for (auto t : tiers)
                            {
                                auto tObj = t.get_object().value();
                                MarginTier tier;
                                tier.lowerBound = toDouble(tObj["lowerBound"].get_string().value());
                                tier.maxLeverage = static_cast<int>(tObj["maxLeverage"].get_int64().value());
                                table.marginTiers.push_back(tier);
                            }

                            dexMeta.marginTables.push_back(std::move(table));
                        }
                    }

                    dexMeta.collateralToken = 0;
                    simdjson::ondemand::value collateralVal;
                    if (metaObj["collateralToken"].get(collateralVal) == simdjson::SUCCESS && !collateralVal.is_null())
                        dexMeta.collateralToken = static_cast<int>(collateralVal.get_int64().value());

                    response.dexMetas.push_back(std::move(dexMeta));
                }
            }
            catch (const simdjson::simdjson_error& e)
            {
                getLogger()->error("RestMessageParser: parse error in allPerpMetas: {}\n  raw: {}", e.what(), message);
            }

            return response;
        }

        PerpDeployAuctionStatusResponse parsePerpDeployAuctionStatus(const std::string& message)
        {
            PerpDeployAuctionStatusResponse response;
            padded = simdjson::padded_string(message.data(), message.size());
            auto doc = parser.iterate(padded);

            try
            {
                auto obj = doc.get_object().value();
                response.startTimeSeconds = obj["startTimeSeconds"].get_uint64().value();
                response.durationSeconds = obj["durationSeconds"].get_uint64().value();
                response.startGas = parseNumberField(obj, "startGas");

                simdjson::ondemand::value currentGasVal;
                if (obj["currentGas"].get(currentGasVal) == simdjson::SUCCESS && !currentGasVal.is_null())
                    response.currentGas = toDouble(currentGasVal.get_string().value());

                simdjson::ondemand::value endGasVal;
                if (obj["endGas"].get(endGasVal) == simdjson::SUCCESS && !endGasVal.is_null())
                    response.endGas = toDouble(endGasVal.get_string().value());
            }
            catch (const simdjson::simdjson_error& err)
            {
                getLogger()->error("RestMessageParser: parse error in perpDeployAuctionStatus: {}\n  raw: {}", err.what(), message);
            }

            return response;
        }

        static double toDouble(std::string_view sv)
        {
            return sv.empty() ? 0.0 : std::stod(std::string(sv));
        }

        // Some numeric info-endpoint fields are wire-encoded as strings, others as
        // raw JSON numbers depending on endpoint/version - accept either.
        static double parseNumberField(simdjson::ondemand::object& obj, std::string_view key)
        {
            auto val = obj[key];
            std::string_view sv;
            if (!val.get_string().get(sv)) return toDouble(sv);
            return val.get_double().value();
        }

        Fill parseFillEntry(simdjson::ondemand::object& obj)
        {
            Fill fill;
            fill.coin = std::string(obj["coin"].get_string().value());
            fill.px = toDouble(obj["px"].get_string().value());
            fill.sz = toDouble(obj["sz"].get_string().value());
            auto sideStr = obj["side"].get_string().value();
            fill.side = sideStr.size() > 0 ? sideStr[0] : '?';
            fill.time = obj["time"].get_uint64().value();
            fill.startPosition = toDouble(obj["startPosition"].get_string().value());
            fill.dir = std::string(obj["dir"].get_string().value());
            fill.closedPnl = toDouble(obj["closedPnl"].get_string().value());
            fill.hash = std::string(obj["hash"].get_string().value());
            fill.oid = obj["oid"].get_uint64().value();
            fill.crossed = obj["crossed"].get_bool().value();
            fill.fee = toDouble(obj["fee"].get_string().value());
            fill.tid = obj["tid"].get_uint64().value();
            fill.feeToken = std::string(obj["feeToken"].get_string().value());

            double builderFee;
            fill.hasBuilderFee = !obj["builderFee"].get_double().get(builderFee);
            if (!fill.hasBuilderFee)
            {
                std::string_view bfStr;
                fill.hasBuilderFee = !obj["builderFee"].get_string().get(bfStr);
                if (fill.hasBuilderFee) builderFee = toDouble(bfStr);
            }
            fill.builderFee = fill.hasBuilderFee ? builderFee : 0.0;

            simdjson::ondemand::object liqObj;
            fill.isLiquidation = !obj["liquidation"].get_object().get(liqObj);
            if (fill.isLiquidation)
            {
                fill.liquidatedUser = std::string(liqObj["liquidatedUser"].get_string().value());
                fill.liquidationMarkPx = toDouble(liqObj["markPx"].get_string().value());
                fill.liquidationMethod = stringToLiquidationMethod(liqObj["method"].get_string().value());
            }
            else
            {
                fill.liquidatedUser.clear();
                fill.liquidationMarkPx = 0.0;
                fill.liquidationMethod = LiquidationMethod::Unknown;
            }

            fill.isSnapshot = false;
            return fill;
        }

        OpenOrder parseOpenOrderEntry(simdjson::ondemand::object& obj)
        {
            OpenOrder order;
            order.coin = std::string(obj["coin"].get_string().value());
            auto sideStr = obj["side"].get_string().value();
            order.side = sideStr.size() > 0 ? sideStr[0] : '?';
            order.limitPx = parseNumberField(obj, "limitPx");
            order.sz = parseNumberField(obj, "sz");
            order.oid = obj["oid"].get_uint64().value();
            order.timestamp = obj["timestamp"].get_uint64().value();
            order.origSz = parseNumberField(obj, "origSz");

            simdjson::ondemand::value cloidVal;
            if (obj["cloid"].get(cloidVal) == simdjson::SUCCESS && !cloidVal.is_null())
                order.cloid = std::string(cloidVal.get_string().value());
            else
                order.cloid.clear();

            return order;
        }

        L2BookResponse parseL2Book(const std::string& message)
        {
            L2BookResponse response;
            response.time = 0;
            padded = simdjson::padded_string(message.data(), message.size());
            auto doc = parser.iterate(padded);

            try
            {
                response.coin = std::string(doc["coin"].get_string().value());
                response.time = doc["time"].get_uint64().value();

                auto levels = doc["levels"].get_array().value();
                size_t sideIdx = 0;
                for (auto side : levels)
                {
                    if (sideIdx > 1)
                    {
                        getLogger()->error("unexpected l2Book side index: {}", sideIdx);
                        break;
                    }
                    auto& target = (sideIdx == 0) ? response.bids : response.asks;
                    auto sideLevels = side.get_array().value();
                    for (auto entry : sideLevels)
                    {
                        auto obj = entry.get_object().value();
                        RestBookLevel level;
                        level.px = std::string(obj["px"].get_string().value());
                        level.sz = std::string(obj["sz"].get_string().value());
                        level.n = static_cast<int>(obj["n"].get_int64().value());
                        target.push_back(std::move(level));
                    }
                    sideIdx++;
                }
            }
            catch (const simdjson::simdjson_error& e)
            {
                getLogger()->error("RestMessageParser: parse error in l2Book: {}\n  raw: {}", e.what(), message);
            }

            return response;
        }

        CandleSnapshotResponse parseCandleSnapshot(const std::string& message)
        {
            CandleSnapshotResponse response;
            padded = simdjson::padded_string(message.data(), message.size());
            auto doc = parser.iterate(padded);

            try
            {
                auto arr = doc.get_array().value();
                for (auto entry : arr)
                {
                    auto obj = entry.get_object().value();
                    Candle candle;
                    candle.coin = std::string(obj["s"].get_string().value());
                    candle.interval = std::string(obj["i"].get_string().value());
                    candle.openTime = obj["t"].get_uint64().value();
                    candle.closeTime = obj["T"].get_uint64().value();
                    candle.open = parseNumberField(obj, "o");
                    candle.close = parseNumberField(obj, "c");
                    candle.high = parseNumberField(obj, "h");
                    candle.low = parseNumberField(obj, "l");
                    candle.volume = parseNumberField(obj, "v");
                    candle.numTrades = static_cast<int>(obj["n"].get_int64().value());
                    response.candles.push_back(std::move(candle));
                }
            }
            catch (const simdjson::simdjson_error& e)
            {
                getLogger()->error("RestMessageParser: parse error in candleSnapshot: {}\n  raw: {}", e.what(), message);
            }

            return response;
        }

        AllMidsResponse parseAllMids(const std::string& message)
        {
            AllMidsResponse response;
            padded = simdjson::padded_string(message.data(), message.size());
            auto doc = parser.iterate(padded);

            try
            {
                auto obj = doc.get_object().value();
                for (auto field : obj)
                {
                    AllMidsEntry entry;
                    entry.coin = std::string(field.unescaped_key().value());
                    entry.mid = toDouble(field.value().get_string().value());
                    response.mids.push_back(std::move(entry));
                }
            }
            catch (const simdjson::simdjson_error& e)
            {
                getLogger()->error("RestMessageParser: parse error in allMids: {}\n  raw: {}", e.what(), message);
            }

            return response;
        }

        OpenOrdersResponse parseOpenOrders(const std::string& message)
        {
            OpenOrdersResponse response;
            padded = simdjson::padded_string(message.data(), message.size());
            auto doc = parser.iterate(padded);

            try
            {
                auto arr = doc.get_array().value();
                for (auto entry : arr)
                {
                    auto obj = entry.get_object().value();
                    response.orders.push_back(parseOpenOrderEntry(obj));
                }
            }
            catch (const simdjson::simdjson_error& e)
            {
                getLogger()->error("RestMessageParser: parse error in openOrders: {}\n  raw: {}", e.what(), message);
            }

            return response;
        }

        OrderStatusResponse parseOrderStatus(const std::string& message)
        {
            OrderStatusResponse response;
            padded = simdjson::padded_string(message.data(), message.size());
            auto doc = parser.iterate(padded);

            try
            {
                response.status = std::string(doc["status"].get_string().value());

                simdjson::ondemand::value orderVal;
                if (doc["order"].get(orderVal) == simdjson::SUCCESS && !orderVal.is_null())
                {
                    auto orderObj = orderVal.get_object().value();
                    OrderStatusOrder result;

                    auto innerOrder = orderObj["order"].get_object().value();
                    result.order = parseOpenOrderEntry(innerOrder);
                    result.status = stringToOrderStatus(orderObj["status"].get_string().value());
                    result.statusTimestamp = orderObj["statusTimestamp"].get_uint64().value();

                    response.order = std::move(result);
                }
            }
            catch (const simdjson::simdjson_error& e)
            {
                getLogger()->error("RestMessageParser: parse error in orderStatus: {}\n  raw: {}", e.what(), message);
            }

            return response;
        }

        UserFillsResponse parseUserFills(const std::string& message)
        {
            UserFillsResponse response;
            padded = simdjson::padded_string(message.data(), message.size());
            auto doc = parser.iterate(padded);

            try
            {
                auto arr = doc.get_array().value();
                for (auto entry : arr)
                {
                    auto obj = entry.get_object().value();
                    response.fills.push_back(parseFillEntry(obj));
                }
            }
            catch (const simdjson::simdjson_error& e)
            {
                getLogger()->error("RestMessageParser: parse error in userFills: {}\n  raw: {}", e.what(), message);
            }

            return response;
        }

        UserFillsResponse parseUserFillsByTime(const std::string& message)
        {
            // Same array-of-fill schema as userFills.
            return parseUserFills(message);
        }

        static BorrowLendReserveState parseBorrowLendReserveStateFields(simdjson::ondemand::object& obj)
        {
            BorrowLendReserveState reserve{};
            reserve.borrowYearlyRate = parseNumberField(obj, "borrowYearlyRate");
            reserve.supplyYearlyRate = parseNumberField(obj, "supplyYearlyRate");
            reserve.balance = parseNumberField(obj, "balance");
            reserve.utilization = parseNumberField(obj, "utilization");
            reserve.oraclePx = parseNumberField(obj, "oraclePx");
            reserve.ltv = parseNumberField(obj, "ltv");
            reserve.totalSupplied = parseNumberField(obj, "totalSupplied");
            reserve.totalBorrowed = parseNumberField(obj, "totalBorrowed");
            return reserve;
        }

        BorrowLendReserveState parseBorrowLendReserveState(const std::string& message)
        {
            BorrowLendReserveState response{};
            padded = simdjson::padded_string(message.data(), message.size());
            auto doc = parser.iterate(padded);

            try
            {
                auto obj = doc.get_object().value();
                response = parseBorrowLendReserveStateFields(obj);
            }
            catch (const simdjson::simdjson_error& err)
            {
                getLogger()->error("RestMessageParser: parse error in borrowLendReserveState: {}\n  raw: {}", err.what(), message);
            }

            return response;
        }

        AllBorrowLendReserveStatesResponse parseAllBorrowLendReserveStates(const std::string& message)
        {
            AllBorrowLendReserveStatesResponse response;
            padded = simdjson::padded_string(message.data(), message.size());
            auto doc = parser.iterate(padded);

            try
            {
                // Top level is an array of [tokenId, reserveStateObj] pairs, not a flat array of objects.
                auto arr = doc.get_array().value();
                for (auto entry : arr)
                {
                    auto pair = entry.get_array().value();
                    auto iter = pair.begin();
                    if (iter == pair.end()) continue;

                    BorrowLendReserveEntry reserveEntry;
                    reserveEntry.token = static_cast<int>((*iter).get_int64().value());
                    ++iter;
                    if (iter == pair.end()) continue;

                    auto obj = (*iter).get_object().value();
                    reserveEntry.state = parseBorrowLendReserveStateFields(obj);

                    response.reserves.push_back(std::move(reserveEntry));
                }
            }
            catch (const simdjson::simdjson_error& err)
            {
                getLogger()->error("RestMessageParser: parse error in allBorrowLendReserveStates: {}\n  raw: {}", err.what(), message);
            }

            return response;
        }

        BorrowLendUserStateResponse parseBorrowLendUserState(const std::string& message)
        {
            BorrowLendUserStateResponse response;
            padded = simdjson::padded_string(message.data(), message.size());
            auto doc = parser.iterate(padded);

            try
            {
                auto obj = doc.get_object().value();

                auto tokenToState = obj["tokenToState"].get_array().value();
                for (auto entry : tokenToState)
                {
                    auto pair = entry.get_array().value();
                    auto iter = pair.begin();
                    if (iter == pair.end()) continue;

                    BorrowLendUserPosition position{};
                    position.token = static_cast<int>((*iter).get_int64().value());
                    ++iter;
                    if (iter == pair.end()) continue;

                    auto posObj = (*iter).get_object().value();

                    auto borrowObj = posObj["borrow"].get_object().value();
                    position.borrow.basis = parseNumberField(borrowObj, "basis");
                    position.borrow.value = parseNumberField(borrowObj, "value");

                    auto supplyObj = posObj["supply"].get_object().value();
                    position.supply.basis = parseNumberField(supplyObj, "basis");
                    position.supply.value = parseNumberField(supplyObj, "value");

                    response.tokenToState.push_back(position);
                }

                response.health = std::string(obj["health"].get_string().value());

                simdjson::ondemand::value healthFactorVal;
                if (obj["healthFactor"].get(healthFactorVal) == simdjson::SUCCESS && !healthFactorVal.is_null())
                {
                    std::string_view sv;
                    response.healthFactor = !healthFactorVal.get_string().get(sv) ? toDouble(sv) : healthFactorVal.get_double().value();
                }
            }
            catch (const simdjson::simdjson_error& err)
            {
                getLogger()->error("RestMessageParser: parse error in borrowLendUserState: {}\n  raw: {}", err.what(), message);
            }

            return response;
        }

        ClearinghouseState parseClearinghouseStateObj(simdjson::ondemand::object& doc)
        {
            ClearinghouseState response{};
            try
            {
                auto marginObj = doc["marginSummary"].get_object().value();
                response.marginSummary.accountValue = toDouble(marginObj["accountValue"].get_string().value());
                response.marginSummary.totalNtlPos = toDouble(marginObj["totalNtlPos"].get_string().value());
                response.marginSummary.totalRawUsd = toDouble(marginObj["totalRawUsd"].get_string().value());
                response.marginSummary.totalMarginUsed = toDouble(marginObj["totalMarginUsed"].get_string().value());

                response.crossMarginSummary = MarginSummary{};
                simdjson::ondemand::value crossVal;
                if (doc["crossMarginSummary"].get(crossVal) == simdjson::SUCCESS && !crossVal.is_null())
                {
                    auto crossObj = crossVal.get_object().value();
                    response.crossMarginSummary.accountValue = toDouble(crossObj["accountValue"].get_string().value());
                    response.crossMarginSummary.totalNtlPos = toDouble(crossObj["totalNtlPos"].get_string().value());
                    response.crossMarginSummary.totalRawUsd = toDouble(crossObj["totalRawUsd"].get_string().value());
                    response.crossMarginSummary.totalMarginUsed = toDouble(crossObj["totalMarginUsed"].get_string().value());
                }

                response.crossMaintenanceMarginUsed = 0.0;
                simdjson::ondemand::value maintVal;
                if (doc["crossMaintenanceMarginUsed"].get(maintVal) == simdjson::SUCCESS && !maintVal.is_null())
                {
                    std::string_view maintSv;
                    if (!maintVal.get_string().get(maintSv)) response.crossMaintenanceMarginUsed = toDouble(maintSv);
                }

                response.withdrawable = toDouble(doc["withdrawable"].get_string().value());
                response.time = doc["time"].get_uint64().value();

                auto positions = doc["assetPositions"].get_array().value();
                for (auto entry : positions)
                {
                    auto obj = entry.get_object().value();
                    auto posObj = obj["position"].get_object().value();
                    AssetPosition pos;
                    pos.coin = std::string(posObj["coin"].get_string().value());
                    pos.szi = toDouble(posObj["szi"].get_string().value());
                    pos.entryPx = toDouble(posObj["entryPx"].get_string().value());
                    pos.positionValue = toDouble(posObj["positionValue"].get_string().value());
                    pos.unrealizedPnl = toDouble(posObj["unrealizedPnl"].get_string().value());
                    pos.returnOnEquity = toDouble(posObj["returnOnEquity"].get_string().value());

                    simdjson::ondemand::value liqPxVal;
                    if (posObj["liquidationPx"].get(liqPxVal) == simdjson::SUCCESS && !liqPxVal.is_null())
                    {
                        std::string_view liqSv;
                        pos.hasLiquidationPx = !liqPxVal.get_string().get(liqSv);
                        pos.liquidationPx = pos.hasLiquidationPx ? toDouble(liqSv) : 0.0;
                    }
                    else
                    {
                        pos.hasLiquidationPx = false;
                        pos.liquidationPx = 0.0;
                    }

                    response.assetPositions.push_back(std::move(pos));
                }
            }
            catch (const simdjson::simdjson_error& e)
            {
                getLogger()->error("RestMessageParser: parse error in clearinghouseState: {}", e.what());
            }

            return response;
        }

        ClearinghouseState parseClearinghouseState(const std::string& message)
        {
            padded = simdjson::padded_string(message.data(), message.size());
            auto doc = parser.iterate(padded);

            try
            {
                auto obj = doc.get_object().value();
                return parseClearinghouseStateObj(obj);
            }
            catch (const simdjson::simdjson_error& e)
            {
                getLogger()->error("RestMessageParser: parse error in clearinghouseState: {}\n  raw: {}", e.what(), message);
            }

            return ClearinghouseState{};
        }

        SpotClearinghouseStateResponse parseSpotClearinghouseStateObj(simdjson::ondemand::object& doc)
        {
            SpotClearinghouseStateResponse response;
            auto balances = doc["balances"].get_array().value();
            for (auto entry : balances)
            {
                auto obj = entry.get_object().value();
                SpotBalance balance;
                balance.coin = std::string(obj["coin"].get_string().value());
                balance.token = static_cast<int>(obj["token"].get_int64().value());
                balance.hold = parseNumberField(obj, "hold");
                balance.total = parseNumberField(obj, "total");
                balance.entryNtl = parseNumberField(obj, "entryNtl");
                response.balances.push_back(std::move(balance));
            }
            return response;
        }

        SpotClearinghouseStateResponse parseSpotClearinghouseState(const std::string& message)
        {
            SpotClearinghouseStateResponse response;
            padded = simdjson::padded_string(message.data(), message.size());
            auto doc = parser.iterate(padded);

            try
            {
                auto obj = doc.get_object().value();
                response = parseSpotClearinghouseStateObj(obj);
            }
            catch (const simdjson::simdjson_error& e)
            {
                getLogger()->error("RestMessageParser: parse error in spotClearinghouseState: {}\n  raw: {}", e.what(), message);
            }

            return response;
        }

        SpotPairDeployAuctionStatusResponse parseSpotPairDeployAuctionStatusObj(simdjson::ondemand::object& obj)
        {
            SpotPairDeployAuctionStatusResponse response;
            response.startTimeSeconds = obj["startTimeSeconds"].get_uint64().value();
            response.durationSeconds = obj["durationSeconds"].get_uint64().value();
            response.startGas = parseNumberField(obj, "startGas");

            simdjson::ondemand::value currentGasVal;
            if (obj["currentGas"].get(currentGasVal) == simdjson::SUCCESS && !currentGasVal.is_null())
                response.currentGas = toDouble(currentGasVal.get_string().value());

            simdjson::ondemand::value endGasVal;
            if (obj["endGas"].get(endGasVal) == simdjson::SUCCESS && !endGasVal.is_null())
                response.endGas = toDouble(endGasVal.get_string().value());

            return response;
        }

        SpotPairDeployAuctionStatusResponse parseSpotPairDeployAuctionStatus(const std::string& message)
        {
            SpotPairDeployAuctionStatusResponse response;
            padded = simdjson::padded_string(message.data(), message.size());
            auto doc = parser.iterate(padded);

            try
            {
                auto obj = doc.get_object().value();
                response = parseSpotPairDeployAuctionStatusObj(obj);
            }
            catch (const simdjson::simdjson_error& e)
            {
                getLogger()->error("RestMessageParser: parse error in spotPairDeployAuctionStatus: {}\n  raw: {}", e.what(), message);
            }

            return response;
        }

        SpotDeployStateResponse parseSpotDeployState(const std::string& message)
        {
            SpotDeployStateResponse response;
            padded = simdjson::padded_string(message.data(), message.size());
            auto doc = parser.iterate(padded);

            try
            {
                auto obj = doc.get_object().value();

                auto states = obj["states"].get_array().value();
                for (auto entry : states)
                {
                    auto stateObj = entry.get_object().value();
                    SpotDeployStateEntry state;
                    state.token = static_cast<int>(stateObj["token"].get_int64().value());

                    auto specObj = stateObj["spec"].get_object().value();
                    state.spec.name = std::string(specObj["name"].get_string().value());
                    state.spec.szDecimals = static_cast<int>(specObj["szDecimals"].get_int64().value());
                    state.spec.weiDecimals = static_cast<int>(specObj["weiDecimals"].get_int64().value());

                    simdjson::ondemand::value fullNameVal;
                    if (stateObj["fullName"].get(fullNameVal) == simdjson::SUCCESS && !fullNameVal.is_null())
                        state.fullName = std::string(fullNameVal.get_string().value());

                    state.deployerTradingFeeShare = parseNumberField(stateObj, "deployerTradingFeeShare");

                    auto spots = stateObj["spots"].get_array().value();
                    for (auto s : spots)
                        state.spots.push_back(static_cast<int>(s.get_int64().value()));

                    simdjson::ondemand::value maxSupplyVal;
                    if (stateObj["maxSupply"].get(maxSupplyVal) == simdjson::SUCCESS && !maxSupplyVal.is_null())
                        state.maxSupply = toDouble(maxSupplyVal.get_string().value());

                    state.hyperliquidityGenesisBalance = parseNumberField(stateObj, "hyperliquidityGenesisBalance");
                    state.totalGenesisBalanceWei = parseNumberField(stateObj, "totalGenesisBalanceWei");

                    auto userGenesisBalances = stateObj["userGenesisBalances"].get_array().value();
                    for (auto balanceEntry : userGenesisBalances)
                    {
                        auto pair = balanceEntry.get_array().value();
                        auto iter = pair.begin();

                        SpotDeployStateGenesisBalance balance;
                        balance.address = std::string((*iter).get_string().value());
                        ++iter;
                        balance.balance = toDouble((*iter).get_string().value());
                        state.userGenesisBalances.push_back(std::move(balance));
                    }

                    auto existingTokenGenesisBalances = stateObj["existingTokenGenesisBalances"].get_array().value();
                    for (auto balanceEntry : existingTokenGenesisBalances)
                    {
                        auto pair = balanceEntry.get_array().value();
                        auto iter = pair.begin();

                        SpotDeployStateExistingTokenBalance balance;
                        balance.token = static_cast<int>((*iter).get_int64().value());
                        ++iter;
                        balance.balance = toDouble((*iter).get_string().value());
                        state.existingTokenGenesisBalances.push_back(std::move(balance));
                    }

                    simdjson::ondemand::array blacklistUsers;
                    if (!stateObj["blacklistUsers"].get_array().get(blacklistUsers))
                    {
                        for (auto u : blacklistUsers)
                            state.blacklistUsers.emplace_back(u.get_string().value());
                    }

                    response.states.push_back(std::move(state));
                }

                auto gasAuctionObj = obj["gasAuction"].get_object().value();
                response.gasAuction = parseSpotPairDeployAuctionStatusObj(gasAuctionObj);
            }
            catch (const simdjson::simdjson_error& e)
            {
                getLogger()->error("RestMessageParser: parse error in spotDeployState: {}\n  raw: {}", e.what(), message);
            }

            return response;
        }

        MetaAndAssetCtxsResponse parseMetaAndAssetCtxs(const std::string& message)
        {
            MetaAndAssetCtxsResponse response;
            padded = simdjson::padded_string(message.data(), message.size());
            auto doc = parser.iterate(padded);

            try
            {
                auto arr = doc.get_array().value();
                auto it = arr.begin();
                if (it == arr.end()) return response;

                auto metaObj = (*it).get_object().value();
                auto universe = metaObj["universe"].get_array().value();
                for (auto entry : universe)
                {
                    auto obj = entry.get_object().value();
                    AssetMeta asset;
                    asset.name = std::string(obj["name"].get_string().value());
                    asset.szDecimals = static_cast<int>(obj["szDecimals"].get_int64().value());
                    asset.maxLeverage = static_cast<int>(obj["maxLeverage"].get_int64().value());
                    response.meta.universe.push_back(std::move(asset));
                }
                ++it;
                if (it == arr.end()) return response;

                auto ctxs = (*it).get_array().value();
                size_t idx = 0;
                for (auto entry : ctxs)
                {
                    auto obj = entry.get_object().value();
                    PerpAssetCtx ctx;
                    ctx.coin = idx < response.meta.universe.size() ? response.meta.universe[idx].name : "";
                    ctx.dayNtlVlm = parseNumberField(obj, "dayNtlVlm");
                    ctx.prevDayPx = parseNumberField(obj, "prevDayPx");
                    ctx.markPx = parseNumberField(obj, "markPx");
                    std::string_view midStr;
                    ctx.hasMidPx = !obj["midPx"].get_string().get(midStr);
                    ctx.midPx = ctx.hasMidPx ? toDouble(midStr) : 0.0;
                    ctx.funding = parseNumberField(obj, "funding");
                    ctx.openInterest = parseNumberField(obj, "openInterest");
                    ctx.oraclePx = parseNumberField(obj, "oraclePx");
                    response.assetCtxs.push_back(std::move(ctx));
                    ++idx;
                }
            }
            catch (const simdjson::simdjson_error& e)
            {
                getLogger()->error("RestMessageParser: parse error in metaAndAssetCtxs: {}\n  raw: {}", e.what(), message);
            }

            return response;
        }

        SpotMetaAndAssetCtxsResponse parseSpotMetaAndAssetCtxs(const std::string& message)
        {
            SpotMetaAndAssetCtxsResponse response;
            padded = simdjson::padded_string(message.data(), message.size());
            auto doc = parser.iterate(padded);

            try
            {
                auto arr = doc.get_array().value();
                auto it = arr.begin();
                if (it == arr.end()) return response;

                auto metaObj = (*it).get_object().value();

                auto tokens = metaObj["tokens"].get_array().value();
                for (auto entry : tokens)
                {
                    auto obj = entry.get_object().value();
                    SpotAssetMeta token;
                    token.name = std::string(obj["name"].get_string().value());
                    token.szDecimals = static_cast<int>(obj["szDecimals"].get_int64().value());
                    token.weiDecimals = static_cast<int>(obj["weiDecimals"].get_int64().value());
                    token.index = static_cast<int>(obj["index"].get_int64().value());
                    token.tokenId = std::string(obj["tokenId"].get_string().value());
                    token.isCanonical = obj["isCanonical"].get_bool().value();
                    simdjson::ondemand::value evmContract;
                    if (obj["evmContract"].get(evmContract) == simdjson::SUCCESS && !evmContract.is_null())
                    {
                        auto address = std::string(evmContract["address"].get_string().value());
                        auto evmExtraWeiDecimals = static_cast<int>(evmContract["evm_extra_wei_decimals"].get_int64().value());
                        token.evmContract = EvmContract{address, evmExtraWeiDecimals};
                    }
                    simdjson::ondemand::value fullName;
                    if (obj["fullName"].get(fullName) == simdjson::SUCCESS && !fullName.is_null())
                        token.fullName = std::string(fullName.get_string().value());
                    response.meta.tokens.push_back(token);
                }

                auto universe = metaObj["universe"].get_array().value();
                for (auto entry : universe)
                {
                    auto obj = entry.get_object().value();
                    SpotUniversePair pair;
                    pair.name = std::string(obj["name"].get_string().value());
                    auto pairTokens = obj["tokens"].get_array().value();
                    for (auto tok : pairTokens)
                        pair.tokens.push_back(static_cast<int>(tok.get_int64().value()));
                    pair.index = static_cast<int>(obj["index"].get_int64().value());
                    pair.isCanonical = obj["isCanonical"].get_bool().value();
                    response.meta.universe.push_back(std::move(pair));
                }
                ++it;
                if (it == arr.end()) return response;

                auto ctxs = (*it).get_array().value();
                size_t idx = 0;
                for (auto entry : ctxs)
                {
                    auto obj = entry.get_object().value();
                    SpotAssetCtx ctx;
                    ctx.coin = idx < response.meta.universe.size() ? response.meta.universe[idx].name : "";
                    ctx.dayNtlVlm = parseNumberField(obj, "dayNtlVlm");
                    ctx.prevDayPx = parseNumberField(obj, "prevDayPx");
                    ctx.markPx = parseNumberField(obj, "markPx");
                    std::string_view midStr;
                    ctx.hasMidPx = !obj["midPx"].get_string().get(midStr);
                    ctx.midPx = ctx.hasMidPx ? toDouble(midStr) : 0.0;
                    simdjson::ondemand::value circSupply;
                    ctx.circulatingSupply = 0.0;
                    if (obj["circulatingSupply"].get(circSupply) == simdjson::SUCCESS && !circSupply.is_null())
                    {
                        std::string_view circSv;
                        ctx.circulatingSupply = !circSupply.get_string().get(circSv) ? toDouble(circSv) : circSupply.get_double().value();
                    }
                    response.assetCtxs.push_back(std::move(ctx));
                    ++idx;
                }
            }
            catch (const simdjson::simdjson_error& e)
            {
                getLogger()->error("RestMessageParser: parse error in spotMetaAndAssetCtxs: {}\n  raw: {}", e.what(), message);
            }

            return response;
        }

        FrontendOrder parseFrontendOrderEntry(simdjson::ondemand::object& obj)
        {
            FrontendOrder order;
            order.coin = std::string(obj["coin"].get_string().value());
            auto sideStr = obj["side"].get_string().value();
            order.side = sideStr.size() > 0 ? sideStr[0] : '?';
            order.limitPx = parseNumberField(obj, "limitPx");
            order.sz = parseNumberField(obj, "sz");
            order.oid = obj["oid"].get_uint64().value();
            order.timestamp = obj["timestamp"].get_uint64().value();
            order.origSz = parseNumberField(obj, "origSz");

            simdjson::ondemand::value cloidVal;
            if (obj["cloid"].get(cloidVal) == simdjson::SUCCESS && !cloidVal.is_null())
                order.cloid = std::string(cloidVal.get_string().value());
            else
                order.cloid.clear();

            order.isPositionTpsl = obj["isPositionTpsl"].get_bool().value();
            order.isTrigger = obj["isTrigger"].get_bool().value();
            order.triggerPx = parseNumberField(obj, "triggerPx");
            order.triggerCondition = std::string(obj["triggerCondition"].get_string().value());
            order.reduceOnly = obj["reduceOnly"].get_bool().value();
            order.orderType = stringToFrontendOrderType(obj["orderType"].get_string().value());

            simdjson::ondemand::value tifVal;
            if (obj["tif"].get(tifVal) == simdjson::SUCCESS && !tifVal.is_null())
                order.tif = stringToOrderTif(tifVal.get_string().value());
            else
                order.tif = std::nullopt;

            return order;
        }

        FrontendOpenOrdersResponse parseFrontendOpenOrders(const std::string& message)
        {
            FrontendOpenOrdersResponse response;
            padded = simdjson::padded_string(message.data(), message.size());
            auto doc = parser.iterate(padded);

            try
            {
                auto arr = doc.get_array().value();
                for (auto entry : arr)
                {
                    auto obj = entry.get_object().value();
                    response.orders.push_back(parseFrontendOrderEntry(obj));
                }
            }
            catch (const simdjson::simdjson_error& e)
            {
                getLogger()->error("RestMessageParser: parse error in frontendOpenOrders: {}\n  raw: {}", e.what(), message);
            }

            return response;
        }

        HistoricalOrdersResponse parseHistoricalOrders(const std::string& message)
        {
            HistoricalOrdersResponse response;
            padded = simdjson::padded_string(message.data(), message.size());
            auto doc = parser.iterate(padded);

            try
            {
                auto arr = doc.get_array().value();
                for (auto entry : arr)
                {
                    auto obj = entry.get_object().value();
                    HistoricalOrder historical;
                    auto orderObj = obj["order"].get_object().value();
                    historical.order = parseFrontendOrderEntry(orderObj);
                    historical.status = stringToOrderStatus(obj["status"].get_string().value());
                    historical.statusTimestamp = obj["statusTimestamp"].get_uint64().value();
                    response.orders.push_back(std::move(historical));
                }
            }
            catch (const simdjson::simdjson_error& e)
            {
                getLogger()->error("RestMessageParser: parse error in historicalOrders: {}\n  raw: {}", e.what(), message);
            }

            return response;
        }

        UserTwapSliceFillsResponse parseUserTwapSliceFills(const std::string& message)
        {
            UserTwapSliceFillsResponse response;
            padded = simdjson::padded_string(message.data(), message.size());
            auto doc = parser.iterate(padded);

            try
            {
                auto arr = doc.get_array().value();
                for (auto entry : arr)
                {
                    auto obj = entry.get_object().value();
                    TwapSliceFill sliceFill;
                    auto fillObj = obj["fill"].get_object().value();
                    sliceFill.fill = parseFillEntry(fillObj);
                    sliceFill.twapId = static_cast<int>(obj["twapId"].get_int64().value());
                    response.fills.push_back(std::move(sliceFill));
                }
            }
            catch (const simdjson::simdjson_error& e)
            {
                getLogger()->error("RestMessageParser: parse error in userTwapSliceFills: {}\n  raw: {}", e.what(), message);
            }

            return response;
        }

        SubAccountsResponse parseSubAccounts(const std::string& message)
        {
            SubAccountsResponse response;
            padded = simdjson::padded_string(message.data(), message.size());
            auto doc = parser.iterate(padded);

            try
            {
                // The API returns a bare `null` rather than `[]` when the user has no sub-accounts.
                if (doc.type().value() == simdjson::ondemand::json_type::null)
                    return response;

                auto arr = doc.get_array().value();
                for (auto entry : arr)
                {
                    auto obj = entry.get_object().value();
                    SubAccount sub;
                    sub.name = std::string(obj["name"].get_string().value());
                    sub.subAccountUser = std::string(obj["subAccountUser"].get_string().value());
                    sub.master = std::string(obj["master"].get_string().value());

                    auto chObj = obj["clearinghouseState"].get_object().value();
                    sub.clearinghouseState = parseClearinghouseStateObj(chObj);

                    auto spotObj = obj["spotState"].get_object().value();
                    sub.spotState = parseSpotClearinghouseStateObj(spotObj);

                    response.subAccounts.push_back(std::move(sub));
                }
            }
            catch (const simdjson::simdjson_error& e)
            {
                getLogger()->error("RestMessageParser: parse error in subAccounts: {}\n  raw: {}", e.what(), message);
            }

            return response;
        }

        UserFeesResponse parseUserFees(const std::string& message)
        {
            UserFeesResponse response{};
            padded = simdjson::padded_string(message.data(), message.size());
            auto doc = parser.iterate(padded);

            try
            {
                auto obj = doc.get_object().value();

                auto dailyVlm = obj["dailyUserVlm"].get_array().value();
                for (auto entry : dailyVlm)
                {
                    auto dObj = entry.get_object().value();
                    DailyUserVolume vol;
                    vol.date = std::string(dObj["date"].get_string().value());
                    vol.userCross = parseNumberField(dObj, "userCross");
                    vol.userAdd = parseNumberField(dObj, "userAdd");
                    vol.exchange = parseNumberField(dObj, "exchange");
                    response.dailyUserVlm.push_back(std::move(vol));
                }

                auto scheduleObj = obj["feeSchedule"].get_object().value();
                response.feeSchedule.cross = parseNumberField(scheduleObj, "cross");
                response.feeSchedule.add = parseNumberField(scheduleObj, "add");
                response.feeSchedule.spotCross = parseNumberField(scheduleObj, "spotCross");
                response.feeSchedule.spotAdd = parseNumberField(scheduleObj, "spotAdd");
                response.feeSchedule.referralDiscount = parseNumberField(scheduleObj, "referralDiscount");

                auto tiersObj = scheduleObj["tiers"].get_object().value();
                auto vipTiers = tiersObj["vip"].get_array().value();
                for (auto entry : vipTiers)
                {
                    auto vObj = entry.get_object().value();
                    FeeTierVip tier;
                    tier.ntlCutoff = parseNumberField(vObj, "ntlCutoff");
                    tier.cross = parseNumberField(vObj, "cross");
                    tier.add = parseNumberField(vObj, "add");
                    tier.spotCross = parseNumberField(vObj, "spotCross");
                    tier.spotAdd = parseNumberField(vObj, "spotAdd");
                    response.feeSchedule.vipTiers.push_back(tier);
                }
                auto mmTiers = tiersObj["mm"].get_array().value();
                for (auto entry : mmTiers)
                {
                    auto mObj = entry.get_object().value();
                    FeeTierMm tier;
                    tier.makerFractionCutoff = parseNumberField(mObj, "makerFractionCutoff");
                    tier.add = parseNumberField(mObj, "add");
                    response.feeSchedule.mmTiers.push_back(tier);
                }

                auto stakingTiers = scheduleObj["stakingDiscountTiers"].get_array().value();
                for (auto entry : stakingTiers)
                {
                    auto sObj = entry.get_object().value();
                    StakingDiscountTier tier;
                    tier.bpsOfMaxSupply = parseNumberField(sObj, "bpsOfMaxSupply");
                    tier.discount = parseNumberField(sObj, "discount");
                    response.feeSchedule.stakingDiscountTiers.push_back(tier);
                }

                response.userCrossRate = parseNumberField(obj, "userCrossRate");
                response.userAddRate = parseNumberField(obj, "userAddRate");
                response.userSpotCrossRate = parseNumberField(obj, "userSpotCrossRate");
                response.userSpotAddRate = parseNumberField(obj, "userSpotAddRate");
                response.activeReferralDiscount = parseNumberField(obj, "activeReferralDiscount");
                response.feeTrialEscrow = parseNumberField(obj, "feeTrialEscrow");

                simdjson::ondemand::value trialTsVal;
                if (obj["nextTrialAvailableTimestamp"].get(trialTsVal) == simdjson::SUCCESS && !trialTsVal.is_null())
                    response.nextTrialAvailableTimestamp = trialTsVal.get_uint64().value();

                simdjson::ondemand::value stakingLinkVal;
                if (obj["stakingLink"].get(stakingLinkVal) == simdjson::SUCCESS && !stakingLinkVal.is_null())
                {
                    auto linkObj = stakingLinkVal.get_object().value();
                    StakingLink link;
                    link.type = std::string(linkObj["type"].get_string().value());
                    link.stakingUser = std::string(linkObj["stakingUser"].get_string().value());
                    response.stakingLink = std::move(link);
                }

                simdjson::ondemand::value activeDiscountVal;
                if (obj["activeStakingDiscount"].get(activeDiscountVal) == simdjson::SUCCESS && !activeDiscountVal.is_null())
                {
                    auto discObj = activeDiscountVal.get_object().value();
                    ActiveStakingDiscount discount;
                    discount.bpsOfMaxSupply = parseNumberField(discObj, "bpsOfMaxSupply");
                    discount.discount = parseNumberField(discObj, "discount");
                    response.activeStakingDiscount = discount;
                }
            }
            catch (const simdjson::simdjson_error& e)
            {
                getLogger()->error("RestMessageParser: parse error in userFees: {}\n  raw: {}", e.what(), message);
            }

            return response;
        }

        MaxBuilderFeeResponse parseMaxBuilderFee(const std::string& message)
        {
            MaxBuilderFeeResponse response{};
            padded = simdjson::padded_string(message.data(), message.size());
            auto doc = parser.iterate(padded);

            try
            {
                response.maxFeeRateTenthsBps = static_cast<int>(doc.get_int64().value());
            }
            catch (const simdjson::simdjson_error& e)
            {
                getLogger()->error("RestMessageParser: parse error in maxBuilderFee: {}\n  raw: {}", e.what(), message);
            }

            return response;
        }

        ApprovedBuildersResponse parseApprovedBuilders(const std::string& message)
        {
            ApprovedBuildersResponse response;
            padded = simdjson::padded_string(message.data(), message.size());
            auto doc = parser.iterate(padded);

            try
            {
                auto arr = doc.get_array().value();
                for (auto entry : arr)
                    response.builders.push_back(std::string(entry.get_string().value()));
            }
            catch (const simdjson::simdjson_error& e)
            {
                getLogger()->error("RestMessageParser: parse error in approvedBuilders: {}\n  raw: {}", e.what(), message);
            }

            return response;
        }

        UserRateLimitResponse parseUserRateLimit(const std::string& message)
        {
            UserRateLimitResponse response{};
            padded = simdjson::padded_string(message.data(), message.size());
            auto doc = parser.iterate(padded);

            try
            {
                response.cumVlm = toDouble(doc["cumVlm"].get_string().value());
                response.nRequestsUsed = doc["nRequestsUsed"].get_int64().value();
                response.nRequestsCap = doc["nRequestsCap"].get_int64().value();
                response.nRequestsSurplus = doc["nRequestsSurplus"].get_int64().value();
            }
            catch (const simdjson::simdjson_error& e)
            {
                getLogger()->error("RestMessageParser: parse error in userRateLimit: {}\n  raw: {}", e.what(), message);
            }

            return response;
        }

        std::vector<PortfolioPeriodMetrics> parsePortfolioArray(simdjson::ondemand::array& arr)
        {
            std::vector<PortfolioPeriodMetrics> periods;
            for (auto entry : arr)
            {
                auto pairArr = entry.get_array().value();
                auto iter = pairArr.begin();
                if (iter == pairArr.end()) continue;
                std::string periodName = std::string((*iter).get_string().value());
                ++iter;
                if (iter == pairArr.end()) continue;
                auto metricsObj = (*iter).get_object().value();

                PortfolioPeriodMetrics metrics;
                metrics.period = stringToPortfolioPeriodType(periodName);

                auto accountValueHistory = metricsObj["accountValueHistory"].get_array().value();
                for (auto point : accountValueHistory)
                {
                    auto pointArr = point.get_array().value();
                    auto pointIter = pointArr.begin();
                    if (pointIter == pointArr.end()) continue;
                    uint64_t time = (*pointIter).get_uint64().value();
                    ++pointIter;
                    if (pointIter == pointArr.end()) continue;
                    double value = toDouble((*pointIter).get_string().value());
                    metrics.accountValueHistory.emplace_back(time, value);
                }

                auto pnlHistory = metricsObj["pnlHistory"].get_array().value();
                for (auto point : pnlHistory)
                {
                    auto pointArr = point.get_array().value();
                    auto pointIter = pointArr.begin();
                    if (pointIter == pointArr.end()) continue;
                    uint64_t time = (*pointIter).get_uint64().value();
                    ++pointIter;
                    if (pointIter == pointArr.end()) continue;
                    double value = toDouble((*pointIter).get_string().value());
                    metrics.pnlHistory.emplace_back(time, value);
                }

                metrics.vlm = parseNumberField(metricsObj, "vlm");
                periods.push_back(std::move(metrics));
            }
            return periods;
        }

        VaultDetailsResponse parseVaultDetails(const std::string& message)
        {
            VaultDetailsResponse response{};
            padded = simdjson::padded_string(message.data(), message.size());
            auto doc = parser.iterate(padded);

            try
            {
                auto obj = doc.get_object().value();
                response.name = std::string(obj["name"].get_string().value());
                response.vaultAddress = std::string(obj["vaultAddress"].get_string().value());
                response.leader = std::string(obj["leader"].get_string().value());
                response.description = std::string(obj["description"].get_string().value());

                auto portfolioArr = obj["portfolio"].get_array().value();
                response.portfolio = parsePortfolioArray(portfolioArr);

                response.apr = parseNumberField(obj, "apr");

                simdjson::ondemand::value followerStateVal;
                if (obj["followerState"].get(followerStateVal) == simdjson::SUCCESS && !followerStateVal.is_null())
                {
                    auto rawStr = simdjson::to_json_string(followerStateVal);
                    if (!rawStr.error()) response.followerStateRaw = std::string(rawStr.value());
                }

                response.leaderFraction = parseNumberField(obj, "leaderFraction");
                response.leaderCommission = parseNumberField(obj, "leaderCommission");

                auto followersArr = obj["followers"].get_array().value();
                for (auto entry : followersArr)
                {
                    auto fObj = entry.get_object().value();
                    VaultFollower follower;
                    follower.user = std::string(fObj["user"].get_string().value());
                    follower.vaultEquity = parseNumberField(fObj, "vaultEquity");
                    follower.pnl = parseNumberField(fObj, "pnl");
                    follower.allTimePnl = parseNumberField(fObj, "allTimePnl");
                    follower.daysFollowing = static_cast<int>(fObj["daysFollowing"].get_int64().value());
                    follower.vaultEntryTime = fObj["vaultEntryTime"].get_uint64().value();
                    follower.lockupUntil = fObj["lockupUntil"].get_uint64().value();
                    response.followers.push_back(std::move(follower));
                }

                response.maxDistributable = parseNumberField(obj, "maxDistributable");
                response.maxWithdrawable = parseNumberField(obj, "maxWithdrawable");
                response.isClosed = obj["isClosed"].get_bool().value();

                simdjson::ondemand::value relVal;
                if (obj["relationship"].get(relVal) == simdjson::SUCCESS && !relVal.is_null())
                {
                    auto relObj = relVal.get_object().value();
                    response.relationship.type = std::string(relObj["type"].get_string().value());
                    simdjson::ondemand::value dataVal;
                    if (relObj["data"].get(dataVal) == simdjson::SUCCESS && !dataVal.is_null())
                    {
                        simdjson::ondemand::array childArr;
                        if (!dataVal["childAddresses"].get_array().get(childArr))
                        {
                            for (auto child : childArr)
                                response.relationship.childAddresses.push_back(std::string(child.get_string().value()));
                        }
                    }
                }

                response.allowDeposits = obj["allowDeposits"].get_bool().value();
                response.alwaysCloseOnWithdraw = obj["alwaysCloseOnWithdraw"].get_bool().value();
            }
            catch (const simdjson::simdjson_error& e)
            {
                getLogger()->error("RestMessageParser: parse error in vaultDetails: {}\n  raw: {}", e.what(), message);
            }

            return response;
        }

        UserVaultEquitiesResponse parseUserVaultEquities(const std::string& message)
        {
            UserVaultEquitiesResponse response;
            padded = simdjson::padded_string(message.data(), message.size());
            auto doc = parser.iterate(padded);

            try
            {
                auto arr = doc.get_array().value();
                for (auto entry : arr)
                {
                    auto obj = entry.get_object().value();
                    UserVaultEquity equity;
                    equity.vaultAddress = std::string(obj["vaultAddress"].get_string().value());
                    equity.equity = parseNumberField(obj, "equity");
                    response.equities.push_back(std::move(equity));
                }
            }
            catch (const simdjson::simdjson_error& e)
            {
                getLogger()->error("RestMessageParser: parse error in userVaultEquities: {}\n  raw: {}", e.what(), message);
            }

            return response;
        }

        PortfolioResponse parsePortfolio(const std::string& message)
        {
            PortfolioResponse response;
            padded = simdjson::padded_string(message.data(), message.size());
            auto doc = parser.iterate(padded);

            try
            {
                auto arr = doc.get_array().value();
                response.periods = parsePortfolioArray(arr);
            }
            catch (const simdjson::simdjson_error& e)
            {
                getLogger()->error("RestMessageParser: parse error in portfolio: {}\n  raw: {}", e.what(), message);
            }

            return response;
        }

        ReferralResponse parseReferral(const std::string& message)
        {
            ReferralResponse response{};
            padded = simdjson::padded_string(message.data(), message.size());
            auto doc = parser.iterate(padded);

            try
            {
                auto obj = doc.get_object().value();

                simdjson::ondemand::value referredByVal;
                if (obj["referredBy"].get(referredByVal) == simdjson::SUCCESS && !referredByVal.is_null())
                {
                    auto rbObj = referredByVal.get_object().value();
                    ReferredBy referredBy;
                    referredBy.referrer = std::string(rbObj["referrer"].get_string().value());
                    referredBy.code = std::string(rbObj["code"].get_string().value());
                    response.referredBy = std::move(referredBy);
                }

                response.cumVlm = parseNumberField(obj, "cumVlm");
                response.unclaimedRewards = parseNumberField(obj, "unclaimedRewards");
                response.claimedRewards = parseNumberField(obj, "claimedRewards");
                response.builderRewards = parseNumberField(obj, "builderRewards");

                simdjson::ondemand::array tokenToStateArr;
                if (!obj["tokenToState"].get_array().get(tokenToStateArr))
                {
                    auto outerIter = tokenToStateArr.begin();
                    if (outerIter != tokenToStateArr.end())
                    {
                        // Wire shape is actually an array containing one [tokenIndex, state]
                        // tuple (e.g. [[0, {...}]]), not a flat 2-element array.
                        auto pairArr = (*outerIter).get_array().value();
                        auto iter = pairArr.begin();
                        if (iter != pairArr.end())
                        {
                            int tokenIndex = static_cast<int>((*iter).get_int64().value());
                            ++iter;
                            if (iter != pairArr.end())
                            {
                                auto stateObj = (*iter).get_object().value();
                                TokenRewardState state;
                                state.cumVlm = parseNumberField(stateObj, "cumVlm");
                                state.unclaimedRewards = parseNumberField(stateObj, "unclaimedRewards");
                                state.claimedRewards = parseNumberField(stateObj, "claimedRewards");
                                state.builderRewards = parseNumberField(stateObj, "builderRewards");
                                response.tokenToState = std::make_pair(tokenIndex, std::move(state));
                            }
                        }
                    }
                }

                simdjson::ondemand::value referrerStateVal;
                if (obj["referrerState"].get(referrerStateVal) == simdjson::SUCCESS && !referrerStateVal.is_null())
                {
                    auto rsObj = referrerStateVal.get_object().value();
                    ReferrerState referrerState;
                    referrerState.stage = std::string(rsObj["stage"].get_string().value());

                    // `data`'s shape is a discriminated union keyed by `stage`. simdjson's
                    // ondemand API requires fields to be accessed in document order and a
                    // speculative lookup for an absent field exhausts the remaining object, so
                    // each stage's fields must be read in isolation rather than speculatively
                    // probed one after another.
                    simdjson::ondemand::object dataObj;
                    if (!rsObj["data"].get_object().get(dataObj))
                    {
                        if (referrerState.stage == "needToTrade")
                        {
                            referrerState.required = parseNumberField(dataObj, "required");
                        }
                        else
                        {
                            std::string_view codeStr;
                            if (!dataObj["code"].get_string().get(codeStr))
                                referrerState.code = std::string(codeStr);

                            simdjson::ondemand::array statesArr;
                            if (!dataObj["referralStates"].get_array().get(statesArr))
                            {
                                for (auto entry : statesArr)
                                {
                                    auto sObj = entry.get_object().value();
                                    ReferralState state;
                                    state.cumVlm = parseNumberField(sObj, "cumVlm");
                                    state.cumRewardedFeesSinceReferred = parseNumberField(sObj, "cumRewardedFeesSinceReferred");
                                    state.cumFeesRewardedToReferrer = parseNumberField(sObj, "cumFeesRewardedToReferrer");
                                    state.timeJoined = sObj["timeJoined"].get_uint64().value();
                                    state.user = std::string(sObj["user"].get_string().value());
                                    referrerState.referralStates.push_back(std::move(state));
                                }
                            }
                        }
                    }
                    response.referrerState = std::move(referrerState);
                }
            }
            catch (const simdjson::simdjson_error& e)
            {
                getLogger()->error("RestMessageParser: parse error in referral: {}\n  raw: {}", e.what(), message);
            }

            return response;
        }

        UserRoleResponse parseUserRole(const std::string& message)
        {
            UserRoleResponse response{};
            padded = simdjson::padded_string(message.data(), message.size());
            auto doc = parser.iterate(padded);

            try
            {
                auto obj = doc.get_object().value();
                response.role = stringToUserRoleType(obj["role"].get_string().value());

                simdjson::ondemand::object dataObj;
                if (!obj["data"].get_object().get(dataObj))
                {
                    if (response.role == UserRoleType::Agent)
                    {
                        std::string_view sv;
                        if (!dataObj["user"].get_string().get(sv)) response.agentUser = std::string(sv);
                    }
                    else if (response.role == UserRoleType::SubAccount)
                    {
                        std::string_view sv;
                        if (!dataObj["master"].get_string().get(sv)) response.subAccountMaster = std::string(sv);
                    }
                }
            }
            catch (const simdjson::simdjson_error& e)
            {
                getLogger()->error("RestMessageParser: parse error in userRole: {}\n  raw: {}", e.what(), message);
            }

            return response;
        }

        SpotMetaResponse parseSpotMeta(const std::string& message)
        {
            SpotMetaResponse response;
            padded = simdjson::padded_string(message.data(), message.size());
            auto doc = parser.iterate(padded);

            try
            {
                auto tokens = doc["tokens"].get_array().value();
                for (auto entry : tokens)
                {
                    auto obj = entry.get_object().value();
                    SpotAssetMeta token;
                    token.name = std::string(obj["name"].get_string().value());
                    token.szDecimals = static_cast<int>(obj["szDecimals"].get_int64().value());
                    token.weiDecimals = static_cast<int>(obj["weiDecimals"].get_int64().value());
                    token.index = static_cast<int>(obj["index"].get_int64().value());
                    token.tokenId = std::string(obj["tokenId"].get_string().value());
                    token.isCanonical = obj["isCanonical"].get_bool().value();
                    simdjson::ondemand::value evmContract;
                    if (obj["evmContract"].get(evmContract) == simdjson::SUCCESS && !evmContract.is_null()) {
                        auto address = std::string(evmContract["address"].get_string().value());
                        auto evmExtraWeiDecimals = static_cast<int>(evmContract["evm_extra_wei_decimals"].get_int64().value());
                        token.evmContract = EvmContract{address, evmExtraWeiDecimals};
                    }
                    simdjson::ondemand::value fullName;
                    if (obj["fullName"].get(fullName) == simdjson::SUCCESS && !fullName.is_null())
                        token.fullName = std::string(fullName.get_string().value());
                    response.tokens.push_back(token);
                }
            }
            catch (const simdjson::simdjson_error& e)
            {
                getLogger()->error("RestMessageParser: parse error in spotMeta: {}\n  raw: {}", e.what(), message);
            }

            return response;
        }

        MetaResponse parseMeta(const std::string& message)
        {
            MetaResponse response;
            padded = simdjson::padded_string(message.data(), message.size());
            auto doc = parser.iterate(padded);

            try
            {
                auto universe = doc["universe"].get_array().value();
                for (auto entry : universe)
                {
                    auto obj = entry.get_object().value();
                    AssetMeta asset;
                    asset.name = std::string(obj["name"].get_string().value());
                    asset.szDecimals = static_cast<int>(obj["szDecimals"].get_int64().value());
                    asset.maxLeverage = static_cast<int>(obj["maxLeverage"].get_int64().value());
                    response.universe.push_back(std::move(asset));
                }
            }
            catch (const simdjson::simdjson_error& e)
            {
                getLogger()->error("RestMessageParser: parse error in meta: {}\n  raw: {}", e.what(), message);
            }

            return response;
        }

    };

    static RestEndpointListener defaultEndpointListener;

    RestApiMessageParser::RestApiMessageParser()
        : impl_(std::make_unique<Impl>(defaultEndpointListener)) {}

    RestApiMessageParser::RestApiMessageParser(RestEndpointListener& listener)
        : impl_(std::make_unique<Impl>(listener)) {}

    RestApiMessageParser::~RestApiMessageParser() = default;
    RestApiMessageParser::RestApiMessageParser(RestApiMessageParser&&) noexcept = default;
    RestApiMessageParser& RestApiMessageParser::operator=(RestApiMessageParser&&) noexcept = default;

    void RestApiMessageParser::parse(const std::string& message, RestEndpointType type,
                                     std::optional<uint64_t> correlationId)
    {
        impl_->parse(message, type, correlationId);
    }

    SpotMetaResponse RestApiMessageParser::parseSpotMeta(const std::string& message)
    {
        return impl_->parseSpotMeta(message);
    }

    MetaResponse RestApiMessageParser::parseMeta(const std::string& message)
    {
        return impl_->parseMeta(message);
    }

    OutcomeMetaResponse RestApiMessageParser::parseOutcomeMeta(const std::string& message)
    {
        return impl_->parseOutcomeMeta(message);
    }

    SettledOutcomeResponse RestApiMessageParser::parseSettledOutcome(const std::string& message)
    {
        return impl_->parseSettledOutcome(message);
    }

    PerpDexsResponse RestApiMessageParser::parsePerpDexs(const std::string& message)
    {
        return impl_->parsePerpDexs(message);
    }

    PerpsAtOpenInterestCapResponse RestApiMessageParser::parsePerpsAtOpenInterestCap(const std::string& message)
    {
        return impl_->parsePerpsAtOpenInterestCap(message);
    }

    PredictedFundingsResponse RestApiMessageParser::parsePredictedFundings(const std::string& message)
    {
        return impl_->parsePredictedFundings(message);
    }

    PerpAnnotationResponse RestApiMessageParser::parsePerpAnnotation(const std::string& message)
    {
        return impl_->parsePerpAnnotation(message);
    }

    PerpCategoriesResponse RestApiMessageParser::parsePerpCategories(const std::string& message)
    {
        return impl_->parsePerpCategories(message);
    }

    PerpConciseAnnotationsResponse RestApiMessageParser::parsePerpConciseAnnotations(const std::string& message)
    {
        return impl_->parsePerpConciseAnnotations(message);
    }

    AllPerpMetasResponse RestApiMessageParser::parseAllPerpMetas(const std::string& message)
    {
        return impl_->parseAllPerpMetas(message);
    }

    PerpDexLimitsResponse RestApiMessageParser::parsePerpDexLimits(const std::string& message)
    {
        return impl_->parsePerpDexLimits(message);
    }

    PerpDexStatusResponse RestApiMessageParser::parsePerpDexStatus(const std::string& message)
    {
        return impl_->parsePerpDexStatus(message);
    }

    PerpDeployAuctionStatusResponse RestApiMessageParser::parsePerpDeployAuctionStatus(const std::string& message)
    {
        return impl_->parsePerpDeployAuctionStatus(message);
    }

    L2BookResponse RestApiMessageParser::parseL2Book(const std::string& message)
    {
        return impl_->parseL2Book(message);
    }

    CandleSnapshotResponse RestApiMessageParser::parseCandleSnapshot(const std::string& message)
    {
        return impl_->parseCandleSnapshot(message);
    }

    AllMidsResponse RestApiMessageParser::parseAllMids(const std::string& message)
    {
        return impl_->parseAllMids(message);
    }

    OpenOrdersResponse RestApiMessageParser::parseOpenOrders(const std::string& message)
    {
        return impl_->parseOpenOrders(message);
    }

    OrderStatusResponse RestApiMessageParser::parseOrderStatus(const std::string& message)
    {
        return impl_->parseOrderStatus(message);
    }

    UserFillsResponse RestApiMessageParser::parseUserFills(const std::string& message)
    {
        return impl_->parseUserFills(message);
    }

    UserFillsResponse RestApiMessageParser::parseUserFillsByTime(const std::string& message)
    {
        return impl_->parseUserFillsByTime(message);
    }

    ClearinghouseState RestApiMessageParser::parseClearinghouseState(const std::string& message)
    {
        return impl_->parseClearinghouseState(message);
    }

    UserRateLimitResponse RestApiMessageParser::parseUserRateLimit(const std::string& message)
    {
        return impl_->parseUserRateLimit(message);
    }

    MetaAndAssetCtxsResponse RestApiMessageParser::parseMetaAndAssetCtxs(const std::string& message)
    {
        return impl_->parseMetaAndAssetCtxs(message);
    }

    SpotMetaAndAssetCtxsResponse RestApiMessageParser::parseSpotMetaAndAssetCtxs(const std::string& message)
    {
        return impl_->parseSpotMetaAndAssetCtxs(message);
    }

    SpotClearinghouseStateResponse RestApiMessageParser::parseSpotClearinghouseState(const std::string& message)
    {
        return impl_->parseSpotClearinghouseState(message);
    }

    SpotDeployStateResponse RestApiMessageParser::parseSpotDeployState(const std::string& message)
    {
        return impl_->parseSpotDeployState(message);
    }

    SpotPairDeployAuctionStatusResponse RestApiMessageParser::parseSpotPairDeployAuctionStatus(const std::string& message)
    {
        return impl_->parseSpotPairDeployAuctionStatus(message);
    }

    FrontendOpenOrdersResponse RestApiMessageParser::parseFrontendOpenOrders(const std::string& message)
    {
        return impl_->parseFrontendOpenOrders(message);
    }

    HistoricalOrdersResponse RestApiMessageParser::parseHistoricalOrders(const std::string& message)
    {
        return impl_->parseHistoricalOrders(message);
    }

    UserTwapSliceFillsResponse RestApiMessageParser::parseUserTwapSliceFills(const std::string& message)
    {
        return impl_->parseUserTwapSliceFills(message);
    }

    SubAccountsResponse RestApiMessageParser::parseSubAccounts(const std::string& message)
    {
        return impl_->parseSubAccounts(message);
    }

    UserFeesResponse RestApiMessageParser::parseUserFees(const std::string& message)
    {
        return impl_->parseUserFees(message);
    }

    MaxBuilderFeeResponse RestApiMessageParser::parseMaxBuilderFee(const std::string& message)
    {
        return impl_->parseMaxBuilderFee(message);
    }

    ApprovedBuildersResponse RestApiMessageParser::parseApprovedBuilders(const std::string& message)
    {
        return impl_->parseApprovedBuilders(message);
    }

    VaultDetailsResponse RestApiMessageParser::parseVaultDetails(const std::string& message)
    {
        return impl_->parseVaultDetails(message);
    }

    UserVaultEquitiesResponse RestApiMessageParser::parseUserVaultEquities(const std::string& message)
    {
        return impl_->parseUserVaultEquities(message);
    }

    PortfolioResponse RestApiMessageParser::parsePortfolio(const std::string& message)
    {
        return impl_->parsePortfolio(message);
    }

    ReferralResponse RestApiMessageParser::parseReferral(const std::string& message)
    {
        return impl_->parseReferral(message);
    }

    UserRoleResponse RestApiMessageParser::parseUserRole(const std::string& message)
    {
        return impl_->parseUserRole(message);
    }

    BorrowLendUserStateResponse RestApiMessageParser::parseBorrowLendUserState(const std::string& message)
    {
        return impl_->parseBorrowLendUserState(message);
    }

    BorrowLendReserveState RestApiMessageParser::parseBorrowLendReserveState(const std::string& message)
    {
        return impl_->parseBorrowLendReserveState(message);
    }

    AllBorrowLendReserveStatesResponse RestApiMessageParser::parseAllBorrowLendReserveStates(const std::string& message)
    {
        return impl_->parseAllBorrowLendReserveStates(message);
    }

    PlaceOrderResponse RestApiMessageParser::parsePlaceOrder(const std::string& message)
    {
        return impl_->parsePlaceOrder(message);
    }

    CancelOrderResponse RestApiMessageParser::parseCancelOrder(const std::string& message)
    {
        return impl_->parseCancelOrder(message);
    }

    ModifyOrderResponse RestApiMessageParser::parseModifyOrder(const std::string& message)
    {
        return impl_->parseModifyOrder(message);
    }

    SimpleResponse RestApiMessageParser::parseSimpleResponse(const std::string& message)
    {
        return impl_->parseSimpleResponse(message);
    }

    TwapOrderResponse RestApiMessageParser::parseTwapOrder(const std::string& message)
    {
        return impl_->parseTwapOrder(message);
    }

    TwapCancelResponse RestApiMessageParser::parseTwapCancel(const std::string& message)
    {
        return impl_->parseTwapCancel(message);
    }

    DelegationsResponse RestApiMessageParser::parseDelegations(const std::string& message)
    {
        return impl_->parseDelegations(message);
    }

    DelegatorSummaryResponse RestApiMessageParser::parseDelegatorSummary(const std::string& message)
    {
        return impl_->parseDelegatorSummary(message);
    }

    DelegatorHistoryResponse RestApiMessageParser::parseDelegatorHistory(const std::string& message)
    {
        return impl_->parseDelegatorHistory(message);
    }

    DelegatorRewardsResponse RestApiMessageParser::parseDelegatorRewards(const std::string& message)
    {
        return impl_->parseDelegatorRewards(message);
    }
} // namespace hyperliquid
