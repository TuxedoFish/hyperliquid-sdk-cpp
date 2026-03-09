#pragma once

#include <string>

namespace hyperliquid {

class WebsocketApiListener {
public:
    virtual ~WebsocketApiListener() = default;

    virtual void onMessage(const std::string& message) {}
    virtual void onConnected() {}
    virtual void onDisconnected(bool hasError, const std::string& errMsg) {}
};

}
