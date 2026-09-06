#include "hyperliquid/websocket/WebsocketMessageHandler.h"
#include "hyperliquid/websocket/WebsocketMessageParser.h"

#include <cstddef>
#include <cstdint>
#include <exception>
#include <string_view>

// libFuzzer harness for WebsocketMessageParser::crack().
//
// Unlike the REST parser, crack() is a single entry point that dispatches on
// the "channel" field found inside the message itself (plus a raw
// pointer-scanning fast path for the common "l2Book" channel that bypasses
// simdjson entirely) - so one fuzz target already exercises every message
// type the parser knows about, including the hand-rolled fast path, which is
// exactly the kind of code most likely to have a memory-safety bug.
//
// Exceptions from the simdjson ondemand path on malformed input are expected
// and swallowed here so libFuzzer's crash detection stays focused on real
// robustness bugs: crashes, ASan/UBSan violations, and hangs.

namespace
{
    class NoOpWebsocketMessageHandler : public hyperliquid::WebsocketMessageHandler
    {
    };

    hyperliquid::WebsocketMessageParser& sharedParser()
    {
        // Long-lived, reused across iterations - mirrors how the SDK uses a
        // single parser instance for the lifetime of a websocket connection,
        // and lets the fuzzer surface state-corruption bugs carried over from
        // one malformed message to the next.
        static hyperliquid::WebsocketMessageParser parser;
        return parser;
    }

    NoOpWebsocketMessageHandler& sharedListener()
    {
        static NoOpWebsocketMessageHandler listener;
        return listener;
    }
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    const std::string_view message(reinterpret_cast<const char*>(data), size);

    try
    {
        sharedParser().crack(message, sharedListener());
    }
    catch (const std::exception&)
    {
    }
    catch (...)
    {
    }

    return 0;
}
