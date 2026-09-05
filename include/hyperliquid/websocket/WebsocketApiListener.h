#pragma once

#include <cstdint>
#include <optional>
#include <string>

#include "hyperliquid/types/RequestTypes.h"

namespace hyperliquid {

class WebsocketApiListener {
public:
    virtual ~WebsocketApiListener() = default;

    virtual void onMessage(const std::string&) {}
    virtual void onPostResponse(const std::string&, RestEndpointType,
                                std::optional<uint64_t> = std::nullopt) {}
    virtual void onConnected() {}
    virtual void onDisconnected(bool, const std::string&) {}
};

}
