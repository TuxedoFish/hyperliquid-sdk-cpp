#include "hyperliquid/rest/RestApiMessageParser.h"
#include <simdjson.h>
#include "Logger.h"

namespace hyperliquid
{
    struct RestApiMessageParser::Impl
    {
        RestEndpointListener& listener;
        simdjson::ondemand::parser parser;
        simdjson::padded_string padded;

        explicit Impl(RestEndpointListener& listener) : listener(listener) {}

        void parse(const std::string& message, RestEndpointType type)
        {
            switch (type)
            {
            case RestEndpointType::SpotMeta:
                listener.onSpotMeta(parseSpotMeta(message));
                break;
            case RestEndpointType::Meta:
                listener.onMeta(parseMeta(message));
                break;
            case RestEndpointType::PerpDexs:
                listener.onPerpDexs(parsePerpDexs(message));
                break;
            case RestEndpointType::PlaceOrder:
                listener.onPlaceOrder(parsePlaceOrder(message));
                break;
            case RestEndpointType::CancelOrder:
            case RestEndpointType::CancelOrderByCloid:
                listener.onCancelOrder(parseCancelOrder(message));
                break;
            case RestEndpointType::ModifyOrder:
            case RestEndpointType::BatchModifyOrder:
                listener.onModifyOrder(parseModifyOrder(message));
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
            }
            catch (const simdjson::simdjson_error& err)
            {
                getLogger()->error("RestMessageParser: parse error in modifyOrder: {}\n  raw: {}", err.what(), message);
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

    void RestApiMessageParser::parse(const std::string& message, RestEndpointType type)
    {
        impl_->parse(message, type);
    }

    SpotMetaResponse RestApiMessageParser::parseSpotMeta(const std::string& message)
    {
        return impl_->parseSpotMeta(message);
    }

    MetaResponse RestApiMessageParser::parseMeta(const std::string& message)
    {
        return impl_->parseMeta(message);
    }

    PerpDexsResponse RestApiMessageParser::parsePerpDexs(const std::string& message)
    {
        return impl_->parsePerpDexs(message);
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
} // namespace hyperliquid
