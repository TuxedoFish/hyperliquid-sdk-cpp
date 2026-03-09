#pragma once

#include "../types/ResponseTypes.h"

namespace hyperliquid {

class RestEndpointListener {
public:
    virtual ~RestEndpointListener() = default;

    virtual void onMeta(const MetaResponse& response) {}
};

}
