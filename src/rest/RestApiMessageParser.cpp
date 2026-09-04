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
            case RestEndpointType::PerpDexs:
                listener.onPerpDexs(parsePerpDexs(message), correlationId);
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
                listener.onSimpleResponse(parseSimpleResponse(message), correlationId);
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
            }

            return response;
        }

        static OutcomeDescription parseOutcomeDescription(const std::string& desc)
        {
            OutcomeDescription result;
            size_t pos = 0;
            while (pos < desc.size())
            {
                size_t sep = desc.find('|', pos);
                std::string_view segment(desc.data() + pos, (sep == std::string::npos ? desc.size() : sep) - pos);
                size_t colon = segment.find(':');
                if (colon != std::string_view::npos)
                {
                    std::string_view key = segment.substr(0, colon);
                    std::string_view val = segment.substr(colon + 1);
                    if (key == "class") result.outcomeClass = std::string(val);
                    else if (key == "underlying") result.underlying = std::string(val);
                    else if (key == "expiry") result.expiry = parseOutcomeExpiry(std::string(val));
                    else if (key == "targetPrice") result.targetPrice = std::string(val);
                    else if (key == "period") result.period = std::string(val);
                }
                if (sep == std::string::npos) break;
                pos = sep + 1;
            }
            return result;
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
                        outcome.sideSpecs.push_back(std::move(spec));
                    }

                    response.outcomes.push_back(std::move(outcome));
                }
            }
            catch (const simdjson::simdjson_error& e)
            {
                getLogger()->error("RestMessageParser: parse error in outcomeMeta: {}\n  raw: {}", e.what(), message);
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
                        std::string asset = std::string((*iter).get_string().value());
                        ++iter;
                        std::string cap = std::string((*iter).get_string().value());
                        dex.assetToStreamingOiCap.emplace_back(asset, cap);
                    }

                    auto fundingMults = obj["assetToFundingMultiplier"].get_array().value();
                    for (auto pair : fundingMults)
                    {
                        auto pairArr = pair.get_array().value();
                        auto iter = pairArr.begin();
                        std::string asset = std::string((*iter).get_string().value());
                        ++iter;
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

        MetaAndAssetCtxsResponse parseMetaAndAssetCtxs(const std::string& message)
        {
            MetaAndAssetCtxsResponse response;
            padded = simdjson::padded_string(message.data(), message.size());
            auto doc = parser.iterate(padded);

            try
            {
                auto arr = doc.get_array().value();
                auto it = arr.begin();

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
                response.feeTrialReward = parseNumberField(obj, "feeTrialReward");

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

    PerpDexsResponse RestApiMessageParser::parsePerpDexs(const std::string& message)
    {
        return impl_->parsePerpDexs(message);
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
} // namespace hyperliquid
