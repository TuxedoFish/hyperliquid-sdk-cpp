#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace hyperliquid
{
    // Low-level, stateless helpers shared by the fast-path (non-simdjson) l2Book parser and the
    // fastAssetCtxs decompression path. No dependency on simdjson or any response type.
    class WebsocketParsingUtils
    {
    public:
        static std::vector<uint8_t> base64Decode(std::string_view input);

        // windowBits=-15 selects raw DEFLATE (RFC 1951): no zlib/gzip header or checksum.
        static bool inflateRawDeflate(const std::vector<uint8_t>& compressed, std::string& out);

        // Find pattern in [p, end), return pointer past the pattern, or nullptr.
        static const char* scanTo(const char* p, const char* end, const char* pattern, size_t len);

        static uint64_t parseUint64Fast(const char*& p, const char* end);
    };
}
