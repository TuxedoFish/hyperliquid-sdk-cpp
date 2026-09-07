#pragma once

#include <cstdint>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>

#include "hyperliquid/types/RequestTypes.h"

namespace hyperliquid
{
    class WebsocketApiListener;

    namespace internal
    {
        struct PostRequestInfo
        {
            RestEndpointType type;
            std::optional<uint64_t> correlationId;
        };

        void handlePostChannelMessage(const std::string& rawMessage,
                                      std::unordered_map<uint64_t, PostRequestInfo>& postRequestInfo,
                                      std::mutex& postRequestInfoMutex,
                                      WebsocketApiListener& listener);
    }
}
