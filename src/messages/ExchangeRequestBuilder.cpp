#include "../messages/ExchangeRequestBuilder.h"

#include <cmath>
#include <cstdio>
#include <stdexcept>
#include <string>

#include "InfoRequestBuilder.h"

namespace hyperliquid
{
    void ExchangeRequestBuilder::initializeMapping(const ApiConfig& config, RestApi* api)
    {
        if (!config.wallet.has_value())
        {
            // Symbol map only needed for authenticated responses
            return;
        }

        auto defaultMetaResponse = api->meta();
        int index = 0;
        for (const auto& asset : defaultMetaResponse.universe)
        {
            symbolMap_.add(asset.name, index);
            index++;
        }

        auto spotMetaResponse = api->spotMeta();
        for (const auto& token : spotMetaResponse.tokens)
        {
            symbolMap_.add(token.name, token.index + 10000);
        }

        if (config.dexes.empty()) return;

        auto dexesResponse = api->perpDexs();
        int perpIdx = 1;
        for (const auto& dex : dexesResponse.dexes)
        {
            if (config.dexes.count(dex.name) == 0)
            {
                perpIdx++;
                continue;
            }

            auto dexMetaResponse = api->meta(dex.name);
            index = 0;
            for (const auto& asset : dexMetaResponse.universe)
            {
                symbolMap_.add(asset.name, 100000 + (perpIdx * 10000) + index);
                index++;
            }
            perpIdx++;
        }
    }


    // Matches Python SDK's float_to_wire: Decimal(f"{x:.8f}").normalize() formatted with :f
    static std::string floatToWire(double x)
    {
        char buf[64];
        std::snprintf(buf, sizeof(buf), "%.8f", x);
        std::string s(buf);

        if (std::abs(std::stod(s) - x) >= 1e-12)
            throw std::invalid_argument("floatToWire causes rounding: " + std::to_string(x));

        // Normalize: strip trailing zeros, then trailing dot
        auto dot = s.find('.');
        if (dot != std::string::npos)
        {
            auto last = s.find_last_not_of('0');
            if (last == dot)
                s = s.substr(0, dot); // integer, drop the dot
            else
                s = s.substr(0, last + 1);
        }
        return s;
    }

    nlohmann::ordered_json ExchangeRequestBuilder::buildOrderWire(const OrderRequest& order) const
    {
        nlohmann::ordered_json orderJson;
        orderJson["a"] = order.assetId
            ? *order.assetId
            : symbolMap_.resolve(order.asset);
        orderJson["b"] = order.isBuy;
        orderJson["p"] = floatToWire(order.price);
        orderJson["s"] = floatToWire(order.size);
        orderJson["r"] = order.reduceOnly;

        if (order.limit)
        {
            nlohmann::ordered_json limitInner;
            limitInner["tif"] = toString(order.limit->tif);
            nlohmann::ordered_json limitOuter;
            limitOuter["limit"] = limitInner;
            orderJson["t"] = limitOuter;
        }
        else if (order.trigger)
        {
            nlohmann::ordered_json triggerInner;
            triggerInner["isMarket"] = order.trigger->isMarket;
            triggerInner["triggerPx"] = floatToWire(order.trigger->triggerPx);
            triggerInner["tpsl"] = toString(order.trigger->tpsl);
            nlohmann::ordered_json triggerOuter;
            triggerOuter["trigger"] = triggerInner;
            orderJson["t"] = triggerOuter;
        }

        if (order.cloid) orderJson["c"] = *order.cloid;

        return orderJson;
    }

    nlohmann::ordered_json ExchangeRequestBuilder::placeOrder(const std::vector<OrderRequest>& orders,
                                                              Grouping grouping,
                                                              const std::optional<Builder>& builder) const
    {
        nlohmann::ordered_json ordersJson = nlohmann::ordered_json::array();
        for (const auto& order : orders)
            ordersJson.push_back(buildOrderWire(order));

        nlohmann::ordered_json action;
        action["type"] = "order";
        action["orders"] = ordersJson;
        action["grouping"] = toString(grouping);

        if (builder)
        {
            nlohmann::ordered_json builderJson;
            builderJson["b"] = builder->address;
            builderJson["f"] = builder->fee;
            action["builder"] = builderJson;
        }

        nlohmann::ordered_json body;
        body["action"] = action;
        return body;
    }

    nlohmann::ordered_json ExchangeRequestBuilder::cancelOrder(const std::vector<CancelRequest>& cancels) const
    {
        nlohmann::ordered_json cancelsJson = nlohmann::ordered_json::array();
        for (const auto& cancel : cancels)
        {
            nlohmann::ordered_json cancelJson;
            cancelJson["a"] = cancel.assetId
                ? *cancel.assetId
                : symbolMap_.resolve(cancel.asset);
            cancelJson["o"] = cancel.oid;
            cancelsJson.push_back(cancelJson);
        }

        nlohmann::ordered_json action;
        action["type"] = "cancel";
        action["cancels"] = cancelsJson;

        nlohmann::ordered_json body;
        body["action"] = action;
        return body;
    }

    nlohmann::ordered_json ExchangeRequestBuilder::cancelOrderByCloid(
        const std::vector<CancelByCloidRequest>& cancels) const
    {
        nlohmann::ordered_json cancelsJson = nlohmann::ordered_json::array();
        for (const auto& cancel : cancels)
        {
            nlohmann::ordered_json cancelJson;
            cancelJson["asset"] = cancel.assetId
                ? *cancel.assetId
                : symbolMap_.resolve(cancel.asset);
            cancelJson["cloid"] = cancel.cloid;
            cancelsJson.push_back(cancelJson);
        }

        nlohmann::ordered_json action;
        action["type"] = "cancelByCloid";
        action["cancels"] = cancelsJson;

        nlohmann::ordered_json body;
        body["action"] = action;
        return body;
    }

    static void setModifyOid(nlohmann::ordered_json& json, const ModifyRequest& modify)
    {
        if (modify.oid)
            json["oid"] = *modify.oid;
        else if (modify.cloid)
            json["oid"] = *modify.cloid;
        else
            throw std::invalid_argument("ModifyRequest requires either oid or cloid");
    }

    nlohmann::ordered_json ExchangeRequestBuilder::modifyOrder(const ModifyRequest& modify) const
    {
        nlohmann::ordered_json action;
        action["type"] = "modify";
        setModifyOid(action, modify);
        action["order"] = buildOrderWire(modify.order);

        nlohmann::ordered_json body;
        body["action"] = action;
        return body;
    }

    nlohmann::ordered_json ExchangeRequestBuilder::batchModifyOrder(const std::vector<ModifyRequest>& modifies) const
    {
        nlohmann::ordered_json modifiesJson = nlohmann::ordered_json::array();
        for (const auto& modify : modifies)
        {
            nlohmann::ordered_json modifyJson;
            setModifyOid(modifyJson, modify);
            modifyJson["order"] = buildOrderWire(modify.order);
            modifiesJson.push_back(modifyJson);
        }

        nlohmann::ordered_json action;
        action["type"] = "batchModify";
        action["modifies"] = modifiesJson;

        nlohmann::ordered_json body;
        body["action"] = action;
        return body;
    }
}
