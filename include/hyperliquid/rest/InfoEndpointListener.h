#pragma once

#include "../types/InfoEndpointTypes.h"

namespace hyperliquid {

class InfoEndpointListener {
public:
    virtual ~InfoEndpointListener() = default;

    virtual void onMeta(const MetaResponse& response) {}
};

}
