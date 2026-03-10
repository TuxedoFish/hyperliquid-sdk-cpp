#include "ExchangeRequestBuilder.h"

#include <cmath>
#include <cstdio>
#include <stdexcept>
#include <string>

namespace hyperliquid {

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

ExchangeRequestBuilder::ExchangeRequestBuilder(const SymbolMap& symbolMap)
    : symbolMap_(symbolMap)
{
}

nlohmann::ordered_json ExchangeRequestBuilder::placeOrder(const std::vector<OrderRequest>& orders,
                                                   Grouping grouping,
                                                   const std::optional<Builder>& builder) const
{
    nlohmann::ordered_json ordersJson = nlohmann::ordered_json::array();
    for (const auto& order : orders)
    {
        nlohmann::ordered_json orderJson;
        orderJson["a"] = symbolMap_.resolve(order.asset);
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

        ordersJson.push_back(orderJson);
    }

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

}
