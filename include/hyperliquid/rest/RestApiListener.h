#pragma once

#include <string>
#include "../types/RequestTypes.h"

namespace hyperliquid {

class RestApiListener {
public:
    virtual ~RestApiListener() = default;

    virtual void onMessage(const std::string&, RestEndpointType) {}
    virtual void onError(RestEndpointType, const std::string&) {}
    virtual void onRateLimitExceeded(RestEndpointType, const std::string&) {}
};

}
