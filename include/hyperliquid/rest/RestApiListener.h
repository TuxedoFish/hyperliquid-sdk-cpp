#pragma once

#include <string>
#include "../types/RequestTypes.h"

namespace hyperliquid {

class RestApiListener {
public:
    virtual ~RestApiListener() = default;

    virtual void onMessage(const std::string& message, RestEndpointType type) {}
    virtual void onError(RestEndpointType type, const std::string& errorMessage) {}
};

}
