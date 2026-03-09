#include "InfoRequestBuilder.h"

namespace hyperliquid {

nlohmann::json InfoRequestBuilder::spotMeta()
{
    nlohmann::json body;
    body["type"] = toString(RestEndpointType::SpotMeta);
    return body;
}

nlohmann::json InfoRequestBuilder::meta(const std::optional<std::string>& dex)
{
    nlohmann::json body;
    body["type"] = toString(RestEndpointType::Meta);
    if (dex) body["dex"] = *dex;
    return body;
}

nlohmann::json InfoRequestBuilder::perpDexs()
{
    nlohmann::json body;
    body["type"] = toString(RestEndpointType::PerpDexs);
    return body;
}

}
