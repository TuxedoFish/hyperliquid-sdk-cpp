#include "hyperliquid/websocket/WebsocketMessageParser.h"
#include <charconv>
#include <iostream>
#include <simdjson.h>

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
                else if (channel == "subscriptionResponse")
                {
                    auto data = doc["data"].get_object().value();
                    crackSubscriptionResponse(data, listener);
                }
                else if (channel == "error")
                {
                    std::cerr << "Error: " << message << std::endl;
                }
                else
                {
                    std::cerr << "Unhandled message: " << message << std::endl;
                }
            }
            catch (const simdjson::simdjson_error& e)
            {
                std::cerr << "parse error: " << e.what() << std::endl;
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
            L2BookUpdate book;
            book.coin = std::string(data["coin"].get_string().value());
            book.time = data["time"].get_uint64().value();

            auto levels = data["levels"].get_array().value();
            size_t sideIdx = 0;
            for (auto side : levels)
            {
                if (sideIdx > 1)
                {
                    std::cerr << "unexpected l2Book side index: " << sideIdx << std::endl;
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
                        listener.onL2BookLevel(book, level);
                    }
                    catch (const simdjson::simdjson_error& e)
                    {
                        std::cerr << "parse error in l2Book level: " << e.what() << std::endl;
                    }
                }
                sideIdx++;
            }
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
                    std::cerr << "parse error in trade: " << e.what() << std::endl;
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
                    std::cerr << "parse error in candle: " << e.what() << std::endl;
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
                    std::cerr << "parse error in allMids entry: " << e.what() << std::endl;
                }
            }
        }

        void crackActiveAssetCtx(simdjson::ondemand::object& data, WebsocketMessageHandler& listener)
        {
            std::string coin(data["coin"].get_string().value());
            auto ctx = data["ctx"].get_object().value();

            simdjson::ondemand::value fundingVal;
            bool isPerp = !ctx["funding"].get(fundingVal);

            if (isPerp)
            {
                PerpAssetCtx perp;
                perp.coin = coin;
                perp.dayNtlVlm = ctx["dayNtlVlm"].get_double().value();
                perp.prevDayPx = ctx["prevDayPx"].get_double().value();
                perp.markPx = ctx["markPx"].get_double().value();
                double midPx;
                perp.hasMidPx = !ctx["midPx"].get_double().get(midPx);
                perp.midPx = perp.hasMidPx ? midPx : 0.0;
                perp.funding = fundingVal.get_double().value();
                perp.openInterest = ctx["openInterest"].get_double().value();
                perp.oraclePx = ctx["oraclePx"].get_double().value();
                listener.onPerpAssetCtx(perp);
            }
            else
            {
                SpotAssetCtx spot;
                spot.coin = coin;
                spot.dayNtlVlm = ctx["dayNtlVlm"].get_double().value();
                spot.prevDayPx = ctx["prevDayPx"].get_double().value();
                spot.markPx = ctx["markPx"].get_double().value();
                double midPx;
                spot.hasMidPx = !ctx["midPx"].get_double().get(midPx);
                spot.midPx = spot.hasMidPx ? midPx : 0.0;
                spot.circulatingSupply = ctx["circulatingSupply"].get_double().value();
                listener.onSpotAssetCtx(spot);
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
} // namespace hyperliquid
