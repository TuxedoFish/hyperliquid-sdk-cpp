#pragma once

#include <string>
#include "../types/RequestTypes.h"

namespace hyperliquid {

class RestListener {
public:
    virtual ~RestListener() = default;

    virtual void onMessage(const std::string& message, InfoEndpointType type) {}
};

}
