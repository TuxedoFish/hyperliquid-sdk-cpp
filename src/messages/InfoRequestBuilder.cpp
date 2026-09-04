#include "../messages/InfoRequestBuilder.h"

namespace hyperliquid {

nlohmann::ordered_json InfoRequestBuilder::spotMeta()
{
    nlohmann::ordered_json body;
    body["type"] = toString(RestEndpointType::SpotMeta);
    return body;
}

nlohmann::ordered_json InfoRequestBuilder::meta(const std::optional<std::string>& dex)
{
    nlohmann::ordered_json body;
    body["type"] = toString(RestEndpointType::Meta);
    if (dex) body["dex"] = *dex;
    return body;
}

nlohmann::ordered_json InfoRequestBuilder::outcomeMeta()
{
    nlohmann::ordered_json body;
    body["type"] = toString(RestEndpointType::OutcomeMeta);
    return body;
}

nlohmann::ordered_json InfoRequestBuilder::perpDexs()
{
    nlohmann::ordered_json body;
    body["type"] = toString(RestEndpointType::PerpDexs);
    return body;
}

nlohmann::ordered_json InfoRequestBuilder::l2Book(const std::string& coin,
                                                  const std::optional<int>& nSigFigs,
                                                  const std::optional<int>& mantissa)
{
    nlohmann::ordered_json body;
    body["type"] = toString(RestEndpointType::L2Book);
    body["coin"] = coin;
    if (nSigFigs) body["nSigFigs"] = *nSigFigs;
    if (mantissa) body["mantissa"] = *mantissa;
    return body;
}

nlohmann::ordered_json InfoRequestBuilder::candleSnapshot(const std::string& coin,
                                                          const std::string& interval,
                                                          uint64_t startTime,
                                                          uint64_t endTime)
{
    nlohmann::ordered_json req;
    req["coin"] = coin;
    req["interval"] = interval;
    req["startTime"] = startTime;
    req["endTime"] = endTime;

    nlohmann::ordered_json body;
    body["type"] = toString(RestEndpointType::CandleSnapshot);
    body["req"] = req;
    return body;
}

nlohmann::ordered_json InfoRequestBuilder::allMids(const std::optional<std::string>& dex)
{
    nlohmann::ordered_json body;
    body["type"] = toString(RestEndpointType::AllMids);
    if (dex) body["dex"] = *dex;
    return body;
}

nlohmann::ordered_json InfoRequestBuilder::openOrders(const std::string& user,
                                                      const std::optional<std::string>& dex)
{
    nlohmann::ordered_json body;
    body["type"] = toString(RestEndpointType::OpenOrders);
    body["user"] = user;
    if (dex) body["dex"] = *dex;
    return body;
}

nlohmann::ordered_json InfoRequestBuilder::orderStatus(const std::string& user, const OrderId& oid)
{
    nlohmann::ordered_json body;
    body["type"] = toString(RestEndpointType::OrderStatus);
    body["user"] = user;
    std::visit([&body](const auto& value) { body["oid"] = value; }, oid);
    return body;
}

nlohmann::ordered_json InfoRequestBuilder::userFills(const std::string& user,
                                                     const std::optional<bool>& aggregateByTime,
                                                     const std::optional<std::string>& dex)
{
    nlohmann::ordered_json body;
    body["type"] = toString(RestEndpointType::UserFills);
    body["user"] = user;
    if (aggregateByTime) body["aggregateByTime"] = *aggregateByTime;
    if (dex) body["dex"] = *dex;
    return body;
}

nlohmann::ordered_json InfoRequestBuilder::userFillsByTime(const std::string& user,
                                                           uint64_t startTime,
                                                           const std::optional<uint64_t>& endTime,
                                                           const std::optional<bool>& aggregateByTime,
                                                           const std::optional<std::string>& dex)
{
    nlohmann::ordered_json body;
    body["type"] = toString(RestEndpointType::UserFillsByTime);
    body["user"] = user;
    body["startTime"] = startTime;
    if (endTime) body["endTime"] = *endTime;
    if (aggregateByTime) body["aggregateByTime"] = *aggregateByTime;
    if (dex) body["dex"] = *dex;
    return body;
}

nlohmann::ordered_json InfoRequestBuilder::clearinghouseState(const std::string& user,
                                                              const std::optional<std::string>& dex)
{
    nlohmann::ordered_json body;
    body["type"] = toString(RestEndpointType::ClearinghouseState);
    body["user"] = user;
    if (dex) body["dex"] = *dex;
    return body;
}

}
