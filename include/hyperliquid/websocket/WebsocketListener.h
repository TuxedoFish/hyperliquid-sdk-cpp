#pragma once

#include <string>

namespace hyperliquid {

class WebsocketListener {
public:
    virtual ~WebsocketListener() = default;

    virtual void onMessage(const std::string& message) {}
    virtual void onConnected() {}
    virtual void onDisconnected(bool hasError, const std::string& errMsg) {}
};

}
