#include "hyperliquid/websocket/WebsocketMessageParser.h"
#include <array>
#include <charconv>
#include <cstring>
#include <simdjson.h>
#include "../config/Logger.h"
#include "WebsocketParsingUtils.h"

#include "../../include/hyperliquid/types/ResponseTypes.h"

namespace hyperliquid
{
    static void parseLevels(const char*& p, const char* end,
                            std::array<PriceLevel, L2_BOOK_MAX_LEVELS>& levels,
                            uint8_t& count, Side side)
    {
        while (p < end && count < L2_BOOK_MAX_LEVELS)
        {
            // Find next "px":"
            const char* px = WebsocketParsingUtils::scanTo(p, end, "\"px\":\"", 6);
            if (!px) return;

            // Check if we crossed the ],[ boundary (bid/ask split)
            // by seeing if there's a ],[ between our current position and px
            for (const char* scan = p; scan < px - 1; ++scan)
            {
                if (scan[0] == ']' && scan[1] == ',' && scan[2] == '[')
                {
                    p = scan + 3;
                    return;  // done with this side
                }
            }

            const char* pxEnd = static_cast<const char*>(memchr(px, '"', end - px));
            if (!pxEnd) return;

            const char* sz = WebsocketParsingUtils::scanTo(pxEnd, end, "\"sz\":\"", 6);
            if (!sz) return;
            const char* szEnd = static_cast<const char*>(memchr(sz, '"', end - sz));
            if (!szEnd) return;

            const char* nPos = WebsocketParsingUtils::scanTo(szEnd, end, "\"n\":", 4);
            if (!nPos) return;
            int n = 0;
            while (nPos < end && *nPos >= '0' && *nPos <= '9')
            {
                n = n * 10 + (*nPos - '0');
                nPos++;
            }

            levels[count].side = side;
            levels[count].px = std::string_view(px, pxEnd - px);
            levels[count].sz = std::string_view(sz, szEnd - sz);
            levels[count].n = n;
            count++;

            p = nPos;
        }
    }

    static bool crackL2BookFast(std::string_view msg, WebsocketMessageHandler& listener)
    {
        L2BookSnapshot snapshot;
        snapshot.numBids = 0;
        snapshot.numAsks = 0;

        const char* p = msg.data();
        const char* end = p + msg.size();

        // Find "coin":" — extract coin name up to closing quote
        p = WebsocketParsingUtils::scanTo(p, end, "\"coin\":\"", 8);
        if (!p) return false;
        const char* coinEnd = static_cast<const char*>(memchr(p, '"', end - p));
        if (!coinEnd) return false;
        snapshot.coin = std::string(p, coinEnd - p);

        // Find "time": — parse uint64
        p = WebsocketParsingUtils::scanTo(coinEnd, end, "\"time\":", 7);
        if (!p) return false;
        snapshot.time = WebsocketParsingUtils::parseUint64Fast(p, end);

        // Find start of levels array: "levels":[[
        p = WebsocketParsingUtils::scanTo(p, end, "[[", 2);
        if (!p) return false;

        // Parse bid levels until we hit ],[ boundary
        parseLevels(p, end, snapshot.bids, snapshot.numBids, Side::Bid);

        // If parseLevels exited due to count limit (not finding ],[),
        // we need to skip past the ],[ boundary before parsing asks.
        const char* boundary = WebsocketParsingUtils::scanTo(p, end, "],[", 3);
        if (boundary) p = boundary;

        // Parse ask levels
        parseLevels(p, end, snapshot.asks, snapshot.numAsks, Side::Ask);

        listener.onL2Book(snapshot);
        return true;
    }

    struct WebsocketMessageParser::Impl
    {
        simdjson::ondemand::parser parser;
        simdjson::padded_string padded;

        static double toDouble(std::string_view sv)
        {
            double val = std::numeric_limits<double>::quiet_NaN();
            std::from_chars(sv.data(), sv.data() + sv.size(), val);
            return val;
        }

        void crack(std::string_view message, WebsocketMessageHandler& listener)
        {
            // Fast path: l2Book is the most common message type.
            // Detect via cheap string scan and parse without simdjson.
            if (message.size() > 30 && message.find("\"l2Book\"") != std::string_view::npos)
            {
                if (crackL2BookFast(message, listener))
                    return;
                // Fall through to simdjson if fast parse failed
            }

            padded = simdjson::padded_string(message.data(), message.size());
            auto doc = parser.iterate(padded);

            try
            {
                std::string_view channel;
                auto channelVal = doc["channel"];
                auto channelType = channelVal.type().value();
                if (channelType == simdjson::ondemand::json_type::string)
                {
                    channel = channelVal.get_string().value();
                }
                else if (channelType == simdjson::ondemand::json_type::object)
                {
                    auto obj = channelVal.get_object().value();
                    auto typeStr = obj["type"].get_string();
                    if (typeStr.error()) return;
                    channel = typeStr.value();
                }
                else
                {
                    return;
                }

                if (channel == "l2Book")
                {
                    // Fast path already handled this above — only here as fallback
                    auto data = doc["data"].get_object().value();
                    crackL2Book(data, listener);
                }
                else if (channel == "bbo")
                {
                    auto data = doc["data"].get_object().value();
                    crackBbo(data, listener);
                }
                else if (channel == "trades")
                {
                    auto data = doc["data"].get_array().value();
                    crackTrades(data, listener);
                }
                else if (channel == "candle")
                {
                    auto data = doc["data"].get_array().value();
                    crackCandles(data, listener);
                }
                else if (channel == "allMids")
                {
                    auto data = doc["data"].get_object().value();
                    crackAllMids(data, listener);
                }
                else if (channel == "activeAssetCtx" || channel == "activeSpotAssetCtx")
                {
                    auto data = doc["data"].get_object().value();
                    crackActiveAssetCtx(data, listener);
                }
                else if (channel == "orderUpdates")
                {
                    auto data = doc["data"].get_array().value();
                    crackOrderUpdates(data, listener);
                }
                else if (channel == "userFills")
                {
                    auto data = doc["data"].get_object().value();
                    crackUserFills(data, listener);
                }
                else if (channel == "userEvents")
                {
                    auto data = doc["data"].get_object().value();
                    crackUserEvents(data, listener);
                }
                else if (channel == "subscriptionResponse")
                {
                    auto data = doc["data"].get_object().value();
                    crackSubscriptionResponse(data, listener);
                }
                else if (channel == "userFundings")
                {
                    auto data = doc["data"].get_object().value();
                    crackUserFundings(data, listener);
                }
                else if (channel == "userNonFundingLedgerUpdates")
                {
                    auto data = doc["data"].get_object().value();
                    crackUserNonFundingLedgerUpdates(data, listener);
                }
                else if (channel == "webData3")
                {
                    auto data = doc["data"].get_object().value();
                    crackWebData3(data, listener);
                }
                else if (channel == "clearinghouseState")
                {
                    auto data = doc["data"].get_object().value();
                    crackClearinghouseState(data, listener);
                }
                else if (channel == "openOrders")
                {
                    auto data = doc["data"].get_object().value();
                    crackOpenOrders(data, listener);
                }
                else if (channel == "twapStates")
                {
                    auto data = doc["data"].get_object().value();
                    crackTwapStates(data, listener);
                }
                else if (channel == "notification")
                {
                    auto data = doc["data"].get_object().value();
                    crackNotification(data, listener);
                }
                else if (channel == "userTwapSliceFills")
                {
                    auto data = doc["data"].get_object().value();
                    crackUserTwapSliceFills(data, listener);
                }
                else if (channel == "userTwapHistory")
                {
                    auto data = doc["data"].get_object().value();
                    crackUserTwapHistory(data, listener);
                }
                else if (channel == "activeAssetData")
                {
                    auto data = doc["data"].get_object().value();
                    crackActiveAssetData(data, listener);
                }
                else if (channel == "spotState")
                {
                    auto data = doc["data"].get_object().value();
                    crackSpotState(data, listener);
                }
                else if (channel == "allDexsClearinghouseState")
                {
                    auto data = doc["data"].get_object().value();
                    crackAllDexsClearinghouseState(data, listener);
                }
                else if (channel == "allDexsAssetCtxs")
                {
                    auto data = doc["data"].get_object().value();
                    crackAllDexsAssetCtxs(data, listener);
                }
                else if (channel == "fastAssetCtxs")
                {
                    auto data = doc["data"].get_string().value();
                    crackFastAssetCtxs(data, listener);
                }
                else if (channel == "outcomeMetaUpdates")
                {
                    auto data = doc["data"].get_object().value();
                    crackOutcomeMetaUpdates(data, listener);
                }
                else if (channel == "error")
                {
                    getLogger()->error("Websocket error: {}", message);
                }
                else
                {
                    getLogger()->warn("Unhandled message: {}", message);
                }
            }
            catch (const simdjson::simdjson_error& e)
            {
                getLogger()->error("parse error: {}", e.what());
            }
        }

        void crackSubscriptionResponse(simdjson::ondemand::object& data, WebsocketMessageHandler& listener)
        {
            SubscriptionResponse response;
            if (data["method"].get_string().value() == "subscribe")
            {
                response.method = SubscriptionMethod::Subscribe;
            } else
            {
                response.method = SubscriptionMethod::Unsubscribe;
            }
            Subscription subscription;
            subscription.type = stringToSubscriptionType(std::string(data["subscription"].get_object().value()["type"].get_string().value()));
            response.subscription = subscription;
            listener.onSubscriptionResponse(response);
        }


        void crackL2Book(simdjson::ondemand::object& data, WebsocketMessageHandler& listener)
        {
            L2BookSnapshot snapshot;
            snapshot.coin = std::string(data["coin"].get_string().value());
            snapshot.time = data["time"].get_uint64().value();
            snapshot.numBids = 0;
            snapshot.numAsks = 0;

            auto levels = data["levels"].get_array().value();
            size_t sideIdx = 0;
            for (auto side : levels)
            {
                if (sideIdx > 1)
                {
                    getLogger()->error("unexpected l2Book side index: {}", sideIdx);
                    break;
                }
                Side s = (sideIdx == 0) ? Side::Bid : Side::Ask;
                auto sideLevels = side.get_array().value();
                for (auto entry : sideLevels)
                {
                    try
                    {
                        auto obj = entry.get_object().value();
                        PriceLevel level;
                        level.side = s;
                        level.px = obj["px"].get_string().value();
                        level.sz = obj["sz"].get_string().value();
                        level.n = static_cast<int>(obj["n"].get_int64().value());

                        if (s == Side::Bid && snapshot.numBids < L2_BOOK_MAX_LEVELS)
                        {
                            snapshot.bids[snapshot.numBids++] = level;
                        }
                        else if (s == Side::Ask && snapshot.numAsks < L2_BOOK_MAX_LEVELS)
                        {
                            snapshot.asks[snapshot.numAsks++] = level;
                        }
                    }
                    catch (const simdjson::simdjson_error& e)
                    {
                        getLogger()->error("parse error in l2Book level: {}", e.what());
                    }
                }
                sideIdx++;
            }

            listener.onL2Book(snapshot);
        }

        void crackBbo(simdjson::ondemand::object& data, WebsocketMessageHandler& listener)
        {
            BboUpdate update;
            update.coin = std::string(data["coin"].get_string().value());
            update.time = data["time"].get_uint64().value();
            update.hasBid = false;
            update.hasAsk = false;

            auto bbo = data["bbo"].get_array().value();
            size_t idx = 0;
            for (auto entry : bbo)
            {
                bool isNull = (entry.type().value() == simdjson::ondemand::json_type::null);
                if (idx == 0)
                {
                    update.hasBid = !isNull;
                    if (!isNull)
                    {
                        auto obj = entry.get_object().value();
                        update.bid.px = obj["px"].get_string().value();
                        update.bid.sz = obj["sz"].get_string().value();
                        update.bid.n = static_cast<int>(obj["n"].get_int64().value());
                    }
                }
                else
                {
                    update.hasAsk = !isNull;
                    if (!isNull)
                    {
                        auto obj = entry.get_object().value();
                        update.ask.px = obj["px"].get_string().value();
                        update.ask.sz = obj["sz"].get_string().value();
                        update.ask.n = static_cast<int>(obj["n"].get_int64().value());
                    }
                }
                idx++;
            }

            listener.onBbo(update);
        }

        void crackTrades(simdjson::ondemand::array& data, WebsocketMessageHandler& listener)
        {
            Trade trade;
            for (auto entry : data)
            {
                try
                {
                    auto obj = entry.get_object().value();
                    trade.coin = std::string(obj["coin"].get_string().value());
                    auto sideStr = obj["side"].get_string().value();
                    trade.side = sideStr.size() > 0 ? sideStr[0] : '?';
                    trade.px = obj["px"].get_string().value();
                    trade.sz = obj["sz"].get_string().value();
                    trade.hash = std::string(obj["hash"].get_string().value());
                    trade.time = obj["time"].get_uint64().value();
                    trade.tid = obj["tid"].get_uint64().value();

                    trade.buyer.clear();
                    trade.seller.clear();
                    simdjson::ondemand::array users;
                    if (!obj["users"].get_array().get(users))
                    {
                        size_t ui = 0;
                        for (auto u : users)
                        {
                            if (ui == 0) trade.buyer = std::string(u.get_string().value());
                            else if (ui == 1) trade.seller = std::string(u.get_string().value());
                            ui++;
                        }
                    }

                    listener.onTrade(trade);
                }
                catch (const simdjson::simdjson_error& e)
                {
                    getLogger()->error("parse error in trade: {}", e.what());
                }
            }
        }

        void crackCandles(simdjson::ondemand::array& data, WebsocketMessageHandler& listener)
        {
            Candle candle;
            for (auto entry : data)
            {
                try
                {
                    auto obj = entry.get_object().value();
                    candle.coin = std::string(obj["s"].get_string().value());
                    candle.interval = std::string(obj["i"].get_string().value());
                    candle.openTime = obj["t"].get_uint64().value();
                    candle.closeTime = obj["T"].get_uint64().value();
                    candle.open = obj["o"].get_double().value();
                    candle.close = obj["c"].get_double().value();
                    candle.high = obj["h"].get_double().value();
                    candle.low = obj["l"].get_double().value();
                    candle.volume = obj["v"].get_double().value();
                    candle.numTrades = static_cast<int>(obj["n"].get_int64().value());
                    listener.onCandle(candle);
                }
                catch (const simdjson::simdjson_error& e)
                {
                    getLogger()->error("parse error in candle: {}", e.what());
                }
            }
        }

        void crackAllMids(simdjson::ondemand::object& data, WebsocketMessageHandler& listener)
        {
            auto mids = data["mids"].get_object().value();
            AllMidsEntry entry;
            for (auto field : mids)
            {
                try
                {
                    entry.coin = std::string(field.unescaped_key().value());
                    entry.mid = toDouble(field.value().get_string().value());
                    listener.onAllMidsEntry(entry);
                }
                catch (const simdjson::simdjson_error& e)
                {
                    getLogger()->error("parse error in allMids entry: {}", e.what());
                }
            }
        }

        // Helper: parse a field that may be a string or a double
        double toDoubleField(simdjson::ondemand::object& obj, std::string_view key)
        {
            auto val = obj[key];
            std::string_view sv;
            if (!val.get_string().get(sv))
                return toDouble(sv);
            return val.get_double().value();
        }

        void crackActiveAssetCtx(simdjson::ondemand::object& data, WebsocketMessageHandler& listener)
        {
            std::string coin(data["coin"].get_string().value());
            auto ctx = data["ctx"].get_object().value();

            std::string_view fundingStr;
            bool isPerp = !ctx["funding"].get_string().get(fundingStr);

            if (isPerp)
            {
                PerpAssetCtx perp;
                perp.coin = coin;
                perp.funding = toDouble(fundingStr);
                perp.dayNtlVlm = toDoubleField(ctx, "dayNtlVlm");
                perp.prevDayPx = toDoubleField(ctx, "prevDayPx");
                perp.markPx = toDoubleField(ctx, "markPx");
                std::string_view midStr;
                perp.hasMidPx = !ctx["midPx"].get_string().get(midStr);
                perp.midPx = perp.hasMidPx ? toDouble(midStr) : 0.0;
                perp.openInterest = toDoubleField(ctx, "openInterest");
                perp.oraclePx = toDoubleField(ctx, "oraclePx");
                listener.onPerpAssetCtx(perp);
            }
            else
            {
                SpotAssetCtx spot;
                spot.coin = coin;
                spot.dayNtlVlm = toDoubleField(ctx, "dayNtlVlm");
                spot.prevDayPx = toDoubleField(ctx, "prevDayPx");
                spot.markPx = toDoubleField(ctx, "markPx");
                std::string_view midStr;
                spot.hasMidPx = !ctx["midPx"].get_string().get(midStr);
                spot.midPx = spot.hasMidPx ? toDouble(midStr) : 0.0;
                spot.circulatingSupply = toDoubleField(ctx, "circulatingSupply");
                listener.onSpotAssetCtx(spot);
            }
        }

        Fill parseFillObject(simdjson::ondemand::object& obj, bool isSnapshot)
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

            // Optional builderFee
            double builderFee;
            fill.hasBuilderFee = !obj["builderFee"].get_double().get(builderFee);
            if (!fill.hasBuilderFee)
            {
                // Try string form
                std::string_view bfStr;
                fill.hasBuilderFee = !obj["builderFee"].get_string().get(bfStr);
                if (fill.hasBuilderFee) builderFee = toDouble(bfStr);
            }
            fill.builderFee = fill.hasBuilderFee ? builderFee : 0.0;

            // Optional liquidation
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

            fill.isSnapshot = isSnapshot;
            return fill;
        }

        void crackFill(simdjson::ondemand::object& obj, WebsocketMessageHandler& listener, bool isSnapshot)
        {
            listener.onUserFill(parseFillObject(obj, isSnapshot));
        }

        void crackOrderUpdates(simdjson::ondemand::array& data, WebsocketMessageHandler& listener)
        {
            for (auto entry : data)
            {
                try
                {
                    auto obj = entry.get_object().value();
                    auto order = obj["order"].get_object().value();

                    OrderUpdate update;
                    update.coin = std::string(order["coin"].get_string().value());
                    auto sideStr = order["side"].get_string().value();
                    update.side = sideStr.size() > 0 ? sideStr[0] : '?';
                    update.limitPx = toDouble(order["limitPx"].get_string().value());
                    update.sz = toDouble(order["sz"].get_string().value());
                    update.oid = order["oid"].get_uint64().value();
                    update.timestamp = order["timestamp"].get_uint64().value();
                    update.origSz = toDouble(order["origSz"].get_string().value());

                    std::string_view cloidStr;
                    if (!order["cloid"].get_string().get(cloidStr))
                        update.cloid = std::string(cloidStr);

                    update.status = stringToOrderStatus(obj["status"].get_string().value());
                    update.statusTimestamp = obj["statusTimestamp"].get_uint64().value();

                    listener.onOrderUpdate(update);
                }
                catch (const simdjson::simdjson_error& e)
                {
                    getLogger()->error("parse error in orderUpdate: {}", e.what());
                }
            }
        }

        void crackUserFills(simdjson::ondemand::object& data, WebsocketMessageHandler& listener)
        {
            bool isSnapshot = false;
            auto snapshotVal = data["isSnapshot"];
            if (!snapshotVal.error())
                isSnapshot = snapshotVal.get_bool().value();

            auto fills = data["fills"].get_array().value();
            for (auto entry : fills)
            {
                try
                {
                    auto obj = entry.get_object().value();
                    crackFill(obj, listener, isSnapshot);
                }
                catch (const simdjson::simdjson_error& e)
                {
                    getLogger()->error("parse error in userFill: {}", e.what());
                }
            }
        }

        void crackUserEvents(simdjson::ondemand::object& data, WebsocketMessageHandler& listener)
        {
            // Discriminated union — check which key exists
            simdjson::ondemand::array fillsArr;
            if (!data["fills"].get_array().get(fillsArr))
            {
                for (auto entry : fillsArr)
                {
                    try
                    {
                        auto obj = entry.get_object().value();
                        crackFill(obj, listener, false);
                    }
                    catch (const simdjson::simdjson_error& e)
                    {
                        getLogger()->error("parse error in userEvents fill: {}", e.what());
                    }
                }
                return;
            }

            simdjson::ondemand::object fundingObj;
            if (!data["funding"].get_object().get(fundingObj))
            {
                UserFunding funding;
                funding.time = fundingObj["time"].get_uint64().value();
                funding.coin = std::string(fundingObj["coin"].get_string().value());
                funding.usdc = toDouble(fundingObj["usdc"].get_string().value());
                funding.szi = toDouble(fundingObj["szi"].get_string().value());
                funding.fundingRate = toDouble(fundingObj["fundingRate"].get_string().value());
                listener.onUserFunding(funding);
                return;
            }

            simdjson::ondemand::object liqObj;
            if (!data["liquidation"].get_object().get(liqObj))
            {
                Liquidation liq;
                liq.lid = liqObj["lid"].get_uint64().value();
                liq.liquidator = std::string(liqObj["liquidator"].get_string().value());
                liq.liquidatedUser = std::string(liqObj["liquidatedUser"].get_string().value());
                liq.liquidatedNtlPos = toDouble(liqObj["liquidatedNtlPos"].get_string().value());
                liq.liquidatedAccountValue = toDouble(liqObj["liquidatedAccountValue"].get_string().value());
                listener.onLiquidation(liq);
                return;
            }

            simdjson::ondemand::array cancelArr;
            if (!data["nonUserCancel"].get_array().get(cancelArr))
            {
                for (auto entry : cancelArr)
                {
                    try
                    {
                        auto obj = entry.get_object().value();
                        NonUserCancel cancel;
                        cancel.coin = std::string(obj["coin"].get_string().value());
                        cancel.oid = obj["oid"].get_uint64().value();
                        listener.onNonUserCancel(cancel);
                    }
                    catch (const simdjson::simdjson_error& e)
                    {
                        getLogger()->error("parse error in userEvents nonUserCancel: {}", e.what());
                    }
                }
                return;
            }
        }

        void crackUserFundings(simdjson::ondemand::object& data, WebsocketMessageHandler& listener)
        {
            bool isSnapshot = false;
            bool snapshotVal;
            if (!data["isSnapshot"].get_bool().get(snapshotVal))
                isSnapshot = snapshotVal;

            auto fundings = data["fundings"].get_array().value();
            for (auto entry : fundings)
            {
                try
                {
                    auto obj = entry.get_object().value();
                    UserFunding funding;
                    funding.time = obj["time"].get_uint64().value();
                    funding.coin = std::string(obj["coin"].get_string().value());
                    funding.usdc = toDouble(obj["usdc"].get_string().value());
                    funding.szi = toDouble(obj["szi"].get_string().value());
                    funding.fundingRate = toDouble(obj["fundingRate"].get_string().value());
                    funding.isSnapshot = isSnapshot;
                    listener.onUserFundingUpdate(funding);
                }
                catch (const simdjson::simdjson_error& e)
                {
                    getLogger()->error("parse error in userFundings entry: {}", e.what());
                }
            }
        }

        void crackLedgerDelta(simdjson::ondemand::object& delta, LedgerUpdate& update)
        {
            std::string_view typeStr = delta["type"].get_string().value();
            update.type = stringToLedgerUpdateType(typeStr);

            switch (update.type)
            {
            case LedgerUpdateType::Deposit:
                update.usdc = toDoubleField(delta, "usdc");
                break;
            case LedgerUpdateType::Withdraw:
                update.usdc = toDoubleField(delta, "usdc");
                update.nonce = delta["nonce"].get_uint64().value();
                update.fee = toDoubleField(delta, "fee");
                break;
            case LedgerUpdateType::InternalTransfer:
                update.usdc = toDoubleField(delta, "usdc");
                update.user = std::string(delta["user"].get_string().value());
                update.destination = std::string(delta["destination"].get_string().value());
                update.fee = toDoubleField(delta, "fee");
                break;
            case LedgerUpdateType::SubAccountTransfer:
                update.usdc = toDoubleField(delta, "usdc");
                update.user = std::string(delta["user"].get_string().value());
                update.destination = std::string(delta["destination"].get_string().value());
                break;
            case LedgerUpdateType::Liquidation:
                update.accountValue = toDoubleField(delta, "accountValue");
                update.leverageType = stringToLeverageType(delta["leverageType"].get_string().value());
                {
                    simdjson::ondemand::array positions;
                    if (!delta["liquidatedPositions"].get_array().get(positions))
                    {
                        for (auto posEntry : positions)
                        {
                            auto posObj = posEntry.get_object().value();
                            LiquidatedPosition pos;
                            pos.coin = std::string(posObj["coin"].get_string().value());
                            pos.szi = toDoubleField(posObj, "szi");
                            update.liquidatedPositions.push_back(std::move(pos));
                        }
                    }
                }
                break;
            case LedgerUpdateType::VaultCreate:
            case LedgerUpdateType::VaultDeposit:
            case LedgerUpdateType::VaultDistribution:
                update.vault = std::string(delta["vault"].get_string().value());
                update.usdc = toDoubleField(delta, "usdc");
                break;
            case LedgerUpdateType::VaultWithdraw:
                update.vault = std::string(delta["vault"].get_string().value());
                update.user = std::string(delta["user"].get_string().value());
                update.requestedUsd = toDoubleField(delta, "requestedUsd");
                update.commission = toDoubleField(delta, "commission");
                update.closingCost = toDoubleField(delta, "closingCost");
                update.basis = toDoubleField(delta, "basis");
                update.netWithdrawnUsd = toDoubleField(delta, "netWithdrawnUsd");
                break;
            case LedgerUpdateType::VaultLeaderCommission:
                update.user = std::string(delta["user"].get_string().value());
                update.usdc = toDoubleField(delta, "usdc");
                break;
            case LedgerUpdateType::SpotTransfer:
                update.token = std::string(delta["token"].get_string().value());
                update.amount = toDoubleField(delta, "amount");
                update.usdcValue = toDoubleField(delta, "usdcValue");
                update.user = std::string(delta["user"].get_string().value());
                update.destination = std::string(delta["destination"].get_string().value());
                update.fee = toDoubleField(delta, "fee");
                break;
            case LedgerUpdateType::Send:
                update.user = std::string(delta["user"].get_string().value());
                update.destination = std::string(delta["destination"].get_string().value());
                update.sourceDex = std::string(delta["sourceDex"].get_string().value());
                update.destinationDex = std::string(delta["destinationDex"].get_string().value());
                update.token = std::string(delta["token"].get_string().value());
                update.amount = toDoubleField(delta, "amount");
                update.usdcValue = toDoubleField(delta, "usdcValue");
                update.fee = toDoubleField(delta, "fee");
                update.nativeTokenFee = toDoubleField(delta, "nativeTokenFee");
                update.nonce = delta["nonce"].get_uint64().value();
                update.feeToken = std::string(delta["feeToken"].get_string().value());
                break;
            case LedgerUpdateType::AccountClassTransfer:
                update.usdc = toDoubleField(delta, "usdc");
                update.toPerp = delta["toPerp"].get_bool().value();
                break;
            case LedgerUpdateType::SpotGenesis:
                update.token = std::string(delta["token"].get_string().value());
                update.amount = toDoubleField(delta, "amount");
                break;
            case LedgerUpdateType::RewardsClaim:
                update.amount = toDoubleField(delta, "amount");
                break;
            default:
                getLogger()->warn("unhandled ledger update delta type: {}", std::string(typeStr));
                break;
            }
        }

        void crackUserNonFundingLedgerUpdates(simdjson::ondemand::object& data, WebsocketMessageHandler& listener)
        {
            bool isSnapshot = false;
            bool snapshotVal;
            if (!data["isSnapshot"].get_bool().get(snapshotVal))
                isSnapshot = snapshotVal;

            auto updates = data["nonFundingLedgerUpdates"].get_array().value();
            for (auto entry : updates)
            {
                try
                {
                    auto obj = entry.get_object().value();
                    LedgerUpdate update{};
                    update.time = obj["time"].get_uint64().value();
                    update.hash = std::string(obj["hash"].get_string().value());
                    auto delta = obj["delta"].get_object().value();
                    crackLedgerDelta(delta, update);
                    update.isSnapshot = isSnapshot;
                    listener.onLedgerUpdate(update);
                }
                catch (const simdjson::simdjson_error& e)
                {
                    getLogger()->error("parse error in userNonFundingLedgerUpdates entry: {}", e.what());
                }
            }
        }

        void crackWebData3(simdjson::ondemand::object& data, WebsocketMessageHandler& listener)
        {
            WebData3Update update;

            auto rawStr = simdjson::to_json_string(data);
            if (!rawStr.error())
                update.raw = std::string(rawStr.value());

            simdjson::ondemand::object userState;
            if (!data["userState"].get_object().get(userState))
            {
                std::string_view agentAddress;
                if (!userState["agentAddress"].get_string().get(agentAddress))
                    update.userState.agentAddress = std::string(agentAddress);

                uint64_t agentValidUntil;
                if (!userState["agentValidUntil"].get_uint64().get(agentValidUntil))
                    update.userState.agentValidUntil = agentValidUntil;

                update.userState.serverTime = userState["serverTime"].get_uint64().value();
                update.userState.cumLedger = toDoubleField(userState, "cumLedger");
                update.userState.isVault = userState["isVault"].get_bool().value();
                update.userState.user = std::string(userState["user"].get_string().value());

                bool optOut;
                update.userState.optOutOfSpotDusting =
                    !userState["optOutOfSpotDusting"].get_bool().get(optOut) && optOut;

                bool dexAbstraction;
                update.userState.dexAbstractionEnabled =
                    !userState["dexAbstractionEnabled"].get_bool().get(dexAbstraction) && dexAbstraction;
            }

            simdjson::ondemand::array perpDexStates;
            if (!data["perpDexStates"].get_array().get(perpDexStates))
            {
                for (auto entry : perpDexStates)
                {
                    try
                    {
                        auto obj = entry.get_object().value();
                        PerpDexState state;
                        state.totalVaultEquity = toDoubleField(obj, "totalVaultEquity");

                        simdjson::ondemand::array caps;
                        if (!obj["perpsAtOpenInterestCap"].get_array().get(caps))
                        {
                            for (auto capEntry : caps)
                                state.perpsAtOpenInterestCap.emplace_back(capEntry.get_string().value());
                        }

                        simdjson::ondemand::array vaults;
                        if (!obj["leadingVaults"].get_array().get(vaults))
                        {
                            for (auto vaultEntry : vaults)
                            {
                                auto vaultObj = vaultEntry.get_object().value();
                                LeadingVault vault;
                                vault.address = std::string(vaultObj["address"].get_string().value());
                                vault.name = std::string(vaultObj["name"].get_string().value());
                                state.leadingVaults.push_back(std::move(vault));
                            }
                        }

                        update.perpDexStates.push_back(std::move(state));
                    }
                    catch (const simdjson::simdjson_error& e)
                    {
                        getLogger()->error("parse error in webData3 perpDexState: {}", e.what());
                    }
                }
            }

            listener.onWebData3(update);
        }

        void crackInnerClearinghouseState(simdjson::ondemand::object& obj, ClearinghouseState& state)
        {
            simdjson::ondemand::array positions;
            if (!obj["assetPositions"].get_array().get(positions))
            {
                for (auto entry : positions)
                {
                    try
                    {
                        auto posEntry = entry.get_object().value();
                        auto position = posEntry["position"].get_object().value();

                        AssetPosition pos;
                        pos.coin = std::string(position["coin"].get_string().value());
                        pos.szi = toDoubleField(position, "szi");
                        pos.entryPx = toDoubleField(position, "entryPx");
                        pos.positionValue = toDoubleField(position, "positionValue");
                        pos.unrealizedPnl = toDoubleField(position, "unrealizedPnl");
                        pos.returnOnEquity = toDoubleField(position, "returnOnEquity");
                        pos.marginUsed = toDoubleField(position, "marginUsed");
                        pos.maxLeverage = static_cast<int>(position["maxLeverage"].get_int64().value());

                        std::string_view liqPxStr;
                        pos.hasLiquidationPx = !position["liquidationPx"].get_string().get(liqPxStr)
                            && !liqPxStr.empty();
                        pos.liquidationPx = pos.hasLiquidationPx ? toDouble(liqPxStr) : 0.0;

                        simdjson::ondemand::object leverage;
                        if (!position["leverage"].get_object().get(leverage))
                            pos.leverageType = stringToLeverageType(leverage["type"].get_string().value());

                        state.assetPositions.push_back(std::move(pos));
                    }
                    catch (const simdjson::simdjson_error& e)
                    {
                        getLogger()->error("parse error in clearinghouseState assetPosition: {}", e.what());
                    }
                }
            }

            auto marginSummary = obj["marginSummary"].get_object().value();
            state.marginSummary.accountValue = toDoubleField(marginSummary, "accountValue");
            state.marginSummary.totalNtlPos = toDoubleField(marginSummary, "totalNtlPos");
            state.marginSummary.totalRawUsd = toDoubleField(marginSummary, "totalRawUsd");
            state.marginSummary.totalMarginUsed = toDoubleField(marginSummary, "totalMarginUsed");

            auto crossMarginSummary = obj["crossMarginSummary"].get_object().value();
            state.crossMarginSummary.accountValue = toDoubleField(crossMarginSummary, "accountValue");
            state.crossMarginSummary.totalNtlPos = toDoubleField(crossMarginSummary, "totalNtlPos");
            state.crossMarginSummary.totalRawUsd = toDoubleField(crossMarginSummary, "totalRawUsd");
            state.crossMarginSummary.totalMarginUsed = toDoubleField(crossMarginSummary, "totalMarginUsed");

            state.crossMaintenanceMarginUsed = toDoubleField(obj, "crossMaintenanceMarginUsed");
            state.withdrawable = toDoubleField(obj, "withdrawable");
        }

        void crackClearinghouseState(simdjson::ondemand::object& data, WebsocketMessageHandler& listener)
        {
            ClearinghouseStateUpdate update;
            std::string_view dex;
            if (!data["dex"].get_string().get(dex))
                update.dex = std::string(dex);
            update.user = std::string(data["user"].get_string().value());

            auto inner = data["clearinghouseState"].get_object().value();
            crackInnerClearinghouseState(inner, update.state);

            listener.onClearinghouseState(update);
        }

        void crackOpenOrders(simdjson::ondemand::object& data, WebsocketMessageHandler& listener)
        {
            OpenOrdersUpdate update;
            std::string_view dex;
            if (!data["dex"].get_string().get(dex))
                update.dex = std::string(dex);
            update.user = std::string(data["user"].get_string().value());

            auto orders = data["orders"].get_array().value();
            for (auto entry : orders)
            {
                try
                {
                    auto obj = entry.get_object().value();
                    OpenOrder order;
                    order.coin = std::string(obj["coin"].get_string().value());
                    auto sideStr = obj["side"].get_string().value();
                    order.side = sideStr.size() > 0 ? sideStr[0] : '?';
                    order.limitPx = toDouble(obj["limitPx"].get_string().value());
                    order.sz = toDouble(obj["sz"].get_string().value());
                    order.oid = obj["oid"].get_uint64().value();
                    order.timestamp = obj["timestamp"].get_uint64().value();
                    order.origSz = toDouble(obj["origSz"].get_string().value());

                    std::string_view cloidStr;
                    if (!obj["cloid"].get_string().get(cloidStr))
                        order.cloid = std::string(cloidStr);

                    update.orders.push_back(std::move(order));
                }
                catch (const simdjson::simdjson_error& e)
                {
                    getLogger()->error("parse error in openOrders entry: {}", e.what());
                }
            }

            listener.onOpenOrdersSnapshot(update);
        }

        void crackTwapState(simdjson::ondemand::object& obj, TwapState& state)
        {
            state.coin = std::string(obj["coin"].get_string().value());
            state.user = std::string(obj["user"].get_string().value());
            auto sideStr = obj["side"].get_string().value();
            state.side = sideStr.size() > 0 ? sideStr[0] : '?';
            state.sz = toDoubleField(obj, "sz");
            state.executedSz = toDoubleField(obj, "executedSz");
            state.executedNtl = toDoubleField(obj, "executedNtl");
            state.minutes = static_cast<int>(obj["minutes"].get_int64().value());
            state.reduceOnly = obj["reduceOnly"].get_bool().value();
            state.randomize = obj["randomize"].get_bool().value();
            state.timestamp = obj["timestamp"].get_uint64().value();
        }

        void crackTwapStates(simdjson::ondemand::object& data, WebsocketMessageHandler& listener)
        {
            TwapStatesUpdate update;
            std::string_view dex;
            if (!data["dex"].get_string().get(dex))
                update.dex = std::string(dex);
            update.user = std::string(data["user"].get_string().value());

            auto states = data["states"].get_array().value();
            for (auto entry : states)
            {
                try
                {
                    auto pair = entry.get_array().value();
                    TwapState state{};
                    size_t idx = 0;
                    for (auto item : pair)
                    {
                        if (idx == 0)
                        {
                            state.id = item.get_uint64().value();
                        }
                        else
                        {
                            auto obj = item.get_object().value();
                            crackTwapState(obj, state);
                        }
                        idx++;
                    }
                    update.states.push_back(std::move(state));
                }
                catch (const simdjson::simdjson_error& e)
                {
                    getLogger()->error("parse error in twapStates entry: {}", e.what());
                }
            }

            listener.onTwapStates(update);
        }

        void crackNotification(simdjson::ondemand::object& data, WebsocketMessageHandler& listener)
        {
            Notification notification;
            notification.notification = std::string(data["notification"].get_string().value());
            listener.onNotification(notification);
        }

        void crackUserTwapSliceFills(simdjson::ondemand::object& data, WebsocketMessageHandler& listener)
        {
            bool isSnapshot = false;
            bool snapshotVal;
            if (!data["isSnapshot"].get_bool().get(snapshotVal))
                isSnapshot = snapshotVal;

            auto sliceFills = data["twapSliceFills"].get_array().value();
            for (auto entry : sliceFills)
            {
                try
                {
                    auto obj = entry.get_object().value();
                    auto fillObj = obj["fill"].get_object().value();

                    TwapSliceFill sliceFill;
                    sliceFill.fill = parseFillObject(fillObj, isSnapshot);
                    sliceFill.twapId = obj["twapId"].get_uint64().value();

                    listener.onUserTwapSliceFill(sliceFill);
                }
                catch (const simdjson::simdjson_error& e)
                {
                    getLogger()->error("parse error in userTwapSliceFills entry: {}", e.what());
                }
            }
        }

        void crackUserTwapHistory(simdjson::ondemand::object& data, WebsocketMessageHandler& listener)
        {
            bool isSnapshot = false;
            bool snapshotVal;
            if (!data["isSnapshot"].get_bool().get(snapshotVal))
                isSnapshot = snapshotVal;

            auto history = data["history"].get_array().value();
            for (auto entry : history)
            {
                try
                {
                    auto obj = entry.get_object().value();

                    TwapHistoryEntry hist{};
                    auto stateObj = obj["state"].get_object().value();
                    crackTwapState(stateObj, hist.state);

                    auto statusObj = obj["status"].get_object().value();
                    hist.status = stringToTwapHistoryStatus(statusObj["status"].get_string().value());
                    std::string_view desc;
                    if (!statusObj["description"].get_string().get(desc))
                        hist.description = std::string(desc);

                    hist.time = obj["time"].get_uint64().value();
                    hist.isSnapshot = isSnapshot;

                    listener.onUserTwapHistory(hist);
                }
                catch (const simdjson::simdjson_error& e)
                {
                    getLogger()->error("parse error in userTwapHistory entry: {}", e.what());
                }
            }
        }

        void crackActiveAssetData(simdjson::ondemand::object& data, WebsocketMessageHandler& listener)
        {
            ActiveAssetData result{};
            result.user = std::string(data["user"].get_string().value());
            result.coin = std::string(data["coin"].get_string().value());

            simdjson::ondemand::object leverage;
            if (!data["leverage"].get_object().get(leverage))
                result.leverageType = stringToLeverageType(leverage["type"].get_string().value());

            auto maxTradeSzs = data["maxTradeSzs"].get_array().value();
            size_t idx = 0;
            for (auto v : maxTradeSzs)
            {
                std::string_view sv;
                double val = !v.get_string().get(sv) ? toDouble(sv) : v.get_double().value();
                if (idx == 0) result.maxTradeSzLong = val;
                else if (idx == 1) result.maxTradeSzShort = val;
                idx++;
            }

            auto availableToTrade = data["availableToTrade"].get_array().value();
            idx = 0;
            for (auto v : availableToTrade)
            {
                std::string_view sv;
                double val = !v.get_string().get(sv) ? toDouble(sv) : v.get_double().value();
                if (idx == 0) result.availableToTradeLong = val;
                else if (idx == 1) result.availableToTradeShort = val;
                idx++;
            }

            listener.onActiveAssetData(result);
        }

        void crackSpotState(simdjson::ondemand::object& data, WebsocketMessageHandler& listener)
        {
            SpotStateUpdate update;
            update.user = std::string(data["user"].get_string().value());

            auto spotState = data["spotState"].get_object().value();
            auto balances = spotState["balances"].get_array().value();
            for (auto entry : balances)
            {
                try
                {
                    auto obj = entry.get_object().value();
                    SpotBalance balance;
                    balance.coin = std::string(obj["coin"].get_string().value());
                    balance.token = static_cast<int>(obj["token"].get_int64().value());
                    balance.hold = toDoubleField(obj, "hold");
                    balance.total = toDoubleField(obj, "total");
                    balance.entryNtl = toDoubleField(obj, "entryNtl");
                    update.balances.push_back(std::move(balance));
                }
                catch (const simdjson::simdjson_error& e)
                {
                    getLogger()->error("parse error in spotState balance: {}", e.what());
                }
            }

            listener.onSpotState(update);
        }

        void crackAllDexsClearinghouseState(simdjson::ondemand::object& data, WebsocketMessageHandler& listener)
        {
            AllDexsClearinghouseStateUpdate update;
            update.user = std::string(data["user"].get_string().value());

            auto states = data["clearinghouseStates"].get_array().value();
            for (auto entry : states)
            {
                try
                {
                    auto pair = entry.get_array().value();
                    DexClearinghouseState dexState{};
                    size_t idx = 0;
                    for (auto item : pair)
                    {
                        if (idx == 0)
                        {
                            dexState.dex = std::string(item.get_string().value());
                        }
                        else
                        {
                            auto obj = item.get_object().value();
                            crackInnerClearinghouseState(obj, dexState.state);
                        }
                        idx++;
                    }
                    update.states.push_back(std::move(dexState));
                }
                catch (const simdjson::simdjson_error& e)
                {
                    getLogger()->error("parse error in allDexsClearinghouseState entry: {}", e.what());
                }
            }

            listener.onAllDexsClearinghouseState(update);
        }

        void crackAllDexsAssetCtxs(simdjson::ondemand::object& data, WebsocketMessageHandler& listener)
        {
            AllDexsAssetCtxsUpdate update;

            auto ctxs = data["ctxs"].get_array().value();
            for (auto entry : ctxs)
            {
                try
                {
                    auto pair = entry.get_array().value();
                    DexAssetCtxs dexCtxs{};
                    size_t idx = 0;
                    for (auto item : pair)
                    {
                        if (idx == 0)
                        {
                            dexCtxs.dex = std::string(item.get_string().value());
                        }
                        else
                        {
                            auto ctxArr = item.get_array().value();
                            for (auto ctxEntry : ctxArr)
                            {
                                auto obj = ctxEntry.get_object().value();
                                PerpAssetCtx ctx{};
                                ctx.dayNtlVlm = toDoubleField(obj, "dayNtlVlm");
                                ctx.prevDayPx = toDoubleField(obj, "prevDayPx");
                                ctx.markPx = toDoubleField(obj, "markPx");
                                std::string_view midStr;
                                ctx.hasMidPx = !obj["midPx"].get_string().get(midStr);
                                ctx.midPx = ctx.hasMidPx ? toDouble(midStr) : 0.0;
                                ctx.funding = toDoubleField(obj, "funding");
                                ctx.openInterest = toDoubleField(obj, "openInterest");
                                ctx.oraclePx = toDoubleField(obj, "oraclePx");
                                dexCtxs.ctxs.push_back(ctx);
                            }
                        }
                        idx++;
                    }
                    update.dexs.push_back(std::move(dexCtxs));
                }
                catch (const simdjson::simdjson_error& e)
                {
                    getLogger()->error("parse error in allDexsAssetCtxs entry: {}", e.what());
                }
            }

            listener.onAllDexsAssetCtxs(update);
        }

        void crackFastAssetCtxs(std::string_view base64Data, WebsocketMessageHandler& listener)
        {
            auto compressed = WebsocketParsingUtils::base64Decode(base64Data);
            std::string decompressed;
            if (!WebsocketParsingUtils::inflateRawDeflate(compressed, decompressed))
            {
                getLogger()->error("failed to inflate fastAssetCtxs payload");
                return;
            }

            try
            {
                simdjson::ondemand::parser localParser;
                simdjson::padded_string padded(decompressed);
                auto localDoc = localParser.iterate(padded);
                auto obj = localDoc.get_object().value();
                for (auto field : obj)
                {
                    try
                    {
                        FastAssetCtx ctx{};
                        ctx.coin = std::string(field.unescaped_key().value());
                        auto entry = field.value().get_object().value();

                        std::string_view markStr;
                        ctx.hasMarkPx = !entry["markPx"].get_string().get(markStr);
                        ctx.markPx = ctx.hasMarkPx ? toDouble(markStr) : 0.0;

                        std::string_view midStr;
                        ctx.hasMidPx = !entry["midPx"].get_string().get(midStr);
                        ctx.midPx = ctx.hasMidPx ? toDouble(midStr) : 0.0;

                        listener.onFastAssetCtx(ctx);
                    }
                    catch (const simdjson::simdjson_error& e)
                    {
                        getLogger()->error("parse error in fastAssetCtxs entry: {}", e.what());
                    }
                }
            }
            catch (const simdjson::simdjson_error& e)
            {
                getLogger()->error("parse error in decompressed fastAssetCtxs payload: {}", e.what());
            }
        }

        static void parseOutcomeSpecForUpdate(simdjson::ondemand::object& obj, Outcome& outcome)
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

        static void parseQuestionSpecForUpdate(simdjson::ondemand::object& obj, QuestionSpec& question)
        {
            question.question = static_cast<int>(obj["question"].get_int64().value());
            question.name = std::string(obj["name"].get_string().value());
            question.description = std::string(obj["description"].get_string().value());
            question.fallbackOutcome = static_cast<int>(obj["fallbackOutcome"].get_int64().value());

            auto namedOutcomes = obj["namedOutcomes"].get_array().value();
            for (auto entry : namedOutcomes)
                question.namedOutcomes.push_back(static_cast<int>(entry.get_int64().value()));

            simdjson::ondemand::array settledNamedOutcomes;
            if (!obj["settledNamedOutcomes"].get_array().get(settledNamedOutcomes))
                for (auto entry : settledNamedOutcomes)
                    question.settledNamedOutcomes.push_back(static_cast<int>(entry.get_int64().value()));
        }

        // The wire format is a discriminated union keyed by variant name (outcomeCreated/
        // outcomeSettled/questionUpdated/questionSettled), mirroring the shape of the
        // delegatorHistory delta union - exactly one key is present per message.
        void crackOutcomeMetaUpdates(simdjson::ondemand::object& data, WebsocketMessageHandler& listener)
        {
            OutcomeMetaUpdate update{};

            simdjson::ondemand::value outcomeCreatedVal;
            simdjson::ondemand::value outcomeSettledVal;
            simdjson::ondemand::value questionUpdatedVal;
            simdjson::ondemand::value questionSettledVal;

            try
            {
                if (data["outcomeCreated"].get(outcomeCreatedVal) == simdjson::SUCCESS)
                {
                    update.type = OutcomeMetaUpdateType::OutcomeCreated;
                    auto obj = outcomeCreatedVal.get_object().value();
                    parseOutcomeSpecForUpdate(obj, update.outcome);
                }
                else if (data["outcomeSettled"].get(outcomeSettledVal) == simdjson::SUCCESS)
                {
                    update.type = OutcomeMetaUpdateType::OutcomeSettled;
                    update.settledOutcome = static_cast<int>(outcomeSettledVal.get_int64().value());
                }
                else if (data["questionUpdated"].get(questionUpdatedVal) == simdjson::SUCCESS)
                {
                    update.type = OutcomeMetaUpdateType::QuestionUpdated;
                    auto obj = questionUpdatedVal.get_object().value();
                    parseQuestionSpecForUpdate(obj, update.question);
                }
                else if (data["questionSettled"].get(questionSettledVal) == simdjson::SUCCESS)
                {
                    update.type = OutcomeMetaUpdateType::QuestionSettled;
                    update.settledQuestion = static_cast<int>(questionSettledVal.get_int64().value());
                }
                else
                {
                    update.type = OutcomeMetaUpdateType::Unknown;
                    getLogger()->warn("outcomeMetaUpdates: unrecognized update variant");
                }
            }
            catch (const simdjson::simdjson_error& e)
            {
                getLogger()->error("parse error in outcomeMetaUpdates: {}", e.what());
            }

            listener.onOutcomeMetaUpdate(update);
        }
    };

    WebsocketMessageParser::WebsocketMessageParser() : impl_(std::make_unique<Impl>()) {}

    WebsocketMessageParser::~WebsocketMessageParser() = default;
    WebsocketMessageParser::WebsocketMessageParser(WebsocketMessageParser&&) noexcept = default;
    WebsocketMessageParser& WebsocketMessageParser::operator=(WebsocketMessageParser&&) noexcept = default;

    void WebsocketMessageParser::crack(std::string_view message, WebsocketMessageHandler& listener)
    {
        impl_->crack(message, listener);
    }

    void WebsocketMessageParser::reset()
    {
    }
} // namespace hyperliquid
