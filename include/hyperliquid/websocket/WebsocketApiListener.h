#pragma once

#include <string>

#include "hyperliquid/types/RequestTypes.h"

namespace hyperliquid {

class WebsocketApiListener {
public:
    virtual ~WebsocketApiListener() = default;

    virtual void onMessage(const std::string& message) {}
    virtual void onPostResponse(const std::string& message, RestEndpointType type) {}
    virtual void onConnected() {}
    virtual void onDisconnected(bool hasError, const std::string& errMsg) {}
};

}
