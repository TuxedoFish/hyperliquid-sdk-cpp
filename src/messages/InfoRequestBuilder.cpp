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

}
