#pragma once

#include <cstdint>
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

        // Handles a single "post"-channel websocket message end-to-end:
        //   - looks up the originating request by id in postRequestInfo (erasing the entry)
        //   - always invokes the generic WebsocketApiListener::onPostResponse fallback
        //   - additionally dispatches to the matching typed on<Endpoint>PostResponse (info reads)
        //     or onExchangeActionPostResponse (exchange actions) callback
        //
        // Expects the full raw websocket message text, e.g.
        //   {"channel":"post","data":{"id":1,"response":{"type":"info","payload":{...}}}}
        //
        // Extracted out of WebsocketApi::Impl::onWsMessage so the dispatch logic can be exercised
        // directly in tests without a live websocket connection.
        void handlePostChannelMessage(const std::string& rawMessage,
                                      std::unordered_map<uint64_t, PostRequestInfo>& postRequestInfo,
                                      WebsocketApiListener& listener);
    }
}
