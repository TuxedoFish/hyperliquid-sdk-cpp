#include "ExchangeRequestBuilder.h"

#include <iostream>

namespace hyperliquid {

ExchangeRequestBuilder::ExchangeRequestBuilder(const SymbolMap& symbolMap)
    : symbolMap_(symbolMap)
{
}

nlohmann::json ExchangeRequestBuilder::placeOrder(const std::vector<OrderRequest>& orders,
                                                   Grouping grouping,
                                                   const std::optional<Builder>& builder) const
{
    nlohmann::json ordersJson = nlohmann::json::array();
    for (const auto& order : orders)
    {
        nlohmann::json orderJson;
        orderJson["a"] = symbolMap_.resolve(order.asset);
        orderJson["b"] = order.isBuy;
        orderJson["p"] = order.price;
        orderJson["s"] = order.size;
        orderJson["r"] = order.reduceOnly;

        if (order.limit)
        {
            orderJson["t"] = {{"limit", {{"tif", toString(order.limit->tif)}}}};
        }
        else if (order.trigger)
        {
            orderJson["t"] = {{"trigger", {
                {"isMarket", order.trigger->isMarket},
                {"triggerPx", order.trigger->triggerPx},
                {"tpsl", toString(order.trigger->tpsl)}
            }}};
        }

        if (order.cloid) orderJson["c"] = *order.cloid;

        ordersJson.push_back(orderJson);
    }

    nlohmann::json action;
    action["type"] = "order";
    action["orders"] = ordersJson;
    action["grouping"] = toString(grouping);

    if (builder)
    {
        action["builder"] = {{"b", builder->address}, {"f", builder->fee}};
    }

    nlohmann::json body;
    body["action"] = action;
    return body;
}

}
