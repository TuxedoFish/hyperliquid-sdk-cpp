#pragma once

#include <string>
#include "../types/RequestTypes.h"

namespace hyperliquid {

// Transport-level callback interface for RestApi's async (xAsync) methods: onMessage gives you
// the raw, unparsed HTTP response body. For typed responses instead of raw strings, parse the
// body yourself with RestApiMessageParser, or use RestEndpointListener via RestApiMessageParser's
// listener-based parse() to get per-endpoint typed callbacks.
class RestApiListener {
public:
    virtual ~RestApiListener() = default;

    virtual void onMessage(const std::string&, RestEndpointType) {}
    virtual void onError(RestEndpointType, const std::string&) {}
    virtual void onRateLimitExceeded(RestEndpointType, const std::string&) {}
};

}
