#include "hyperliquid/websocket/WebsocketMessageParser.h"
#include <charconv>
#include <simdjson.h>
#include "../config/Logger.h"

#include "../../include/hyperliquid/types/ResponseTypes.h"

namespace hyperliquid
{
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

        void crack(const std::string& message, WebsocketMessageHandler& listener)
        {
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

            L2BookUpdate book;
            book.coin = snapshot.coin;
            book.time = snapshot.time;

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
                for (auto entry : side.get_array().value())
                {
                    try
                    {
                        auto obj = entry.get_object().value();
                        PriceLevel level;
                        level.side = s;
                        level.px = std::string(obj["px"].get_string().value());
                        level.sz = std::string(obj["sz"].get_string().value());
                        level.n = static_cast<int>(obj["n"].get_int64().value());

                        if (s == Side::Bid && snapshot.numBids < L2_BOOK_MAX_LEVELS)
                        {
                            snapshot.bids[snapshot.numBids++] = level;
                        }
                        else if (s == Side::Ask && snapshot.numAsks < L2_BOOK_MAX_LEVELS)
                        {
                            snapshot.asks[snapshot.numAsks++] = level;
                        }

                        listener.onL2BookLevel(book, level);
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
                        update.bid.px = std::string(obj["px"].get_string().value());
                        update.bid.sz = std::string(obj["sz"].get_string().value());
                        update.bid.n = static_cast<int>(obj["n"].get_int64().value());
                    }
                }
                else
                {
                    update.hasAsk = !isNull;
                    if (!isNull)
                    {
                        auto obj = entry.get_object().value();
                        update.ask.px = std::string(obj["px"].get_string().value());
                        update.ask.sz = std::string(obj["sz"].get_string().value());
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
                    trade.px = std::string(obj["px"].get_string().value());
                    trade.sz = std::string(obj["sz"].get_string().value());
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

        void crackFill(simdjson::ondemand::object& obj, WebsocketMessageHandler& listener, bool isSnapshot)
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
            listener.onUserFill(fill);
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
    };

    WebsocketMessageParser::WebsocketMessageParser() : impl_(std::make_unique<Impl>()) {}

    WebsocketMessageParser::~WebsocketMessageParser() = default;
    WebsocketMessageParser::WebsocketMessageParser(WebsocketMessageParser&&) noexcept = default;
    WebsocketMessageParser& WebsocketMessageParser::operator=(WebsocketMessageParser&&) noexcept = default;

    void WebsocketMessageParser::crack(const std::string& message, WebsocketMessageHandler& listener)
    {
        impl_->crack(message, listener);
    }

    void WebsocketMessageParser::reset()
    {
    }
} // namespace hyperliquid
