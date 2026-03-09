#pragma once

#include <map>
#include <memory>
#include <string>

#include "../types/RequestTypes.h"
#include "RestApiListener.h"
#include "hyperliquid/signing/Wallet.h"

namespace hyperliquid {

class RestApi {
public:
    RestApi(Environment env, RestApiListener& listener, Wallet wallet);
    RestApi(Environment env, RestApiListener& listener);
    ~RestApi();

    RestApi(const RestApi&) = delete;
    RestApi& operator=(const RestApi&) = delete;

    void sendRequest(RestEndpointType type, const std::map<std::string, std::string>& params = {});

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace hyperliquid
