#pragma once

#include <map>
#include <memory>
#include <string>

#include "../types/RequestTypes.h"
#include "RestListener.h"

namespace hyperliquid {

class InfoApi {
public:
    InfoApi(Environment env, RestListener& listener);
    ~InfoApi();

    InfoApi(const InfoApi&) = delete;
    InfoApi& operator=(const InfoApi&) = delete;

    void sendRequest(InfoEndpointType type,
                     const std::map<std::string, std::string>& params = {});

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace hyperliquid
