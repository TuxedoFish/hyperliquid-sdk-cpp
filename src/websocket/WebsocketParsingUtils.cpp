#include "WebsocketParsingUtils.h"

#include <array>
#include <cstring>
#include <zlib.h>

namespace hyperliquid
{
    std::vector<uint8_t> WebsocketParsingUtils::base64Decode(std::string_view input)
    {
        static int8_t table[256];
        static bool initialized = false;
        if (!initialized)
        {
            std::fill(std::begin(table), std::end(table), int8_t{-1});
            static const char* alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
            for (int i = 0; i < 64; ++i)
                table[static_cast<uint8_t>(alphabet[i])] = static_cast<int8_t>(i);
            initialized = true;
        }

        std::vector<uint8_t> out;
        out.reserve(input.size() / 4 * 3);
        int val = 0, bits = -8;
        for (unsigned char c : input)
        {
            if (table[c] == -1) continue;
            val = (val << 6) + table[c];
            bits += 6;
            if (bits >= 0)
            {
                out.push_back(static_cast<uint8_t>((val >> bits) & 0xFF));
                bits -= 8;
            }
        }
        return out;
    }

    bool WebsocketParsingUtils::inflateRawDeflate(const std::vector<uint8_t>& compressed, std::string& out)
    {
        z_stream stream{};
        if (inflateInit2(&stream, -15) != Z_OK) return false;

        stream.next_in = const_cast<Bytef*>(compressed.data());
        stream.avail_in = static_cast<uInt>(compressed.size());

        std::array<char, 4096> buffer{};
        int ret;
        do
        {
            stream.next_out = reinterpret_cast<Bytef*>(buffer.data());
            stream.avail_out = static_cast<uInt>(buffer.size());
            ret = inflate(&stream, Z_NO_FLUSH);
            if (ret != Z_OK && ret != Z_STREAM_END && ret != Z_BUF_ERROR)
            {
                inflateEnd(&stream);
                return false;
            }
            out.append(buffer.data(), buffer.size() - stream.avail_out);
        } while (ret != Z_STREAM_END && ret != Z_BUF_ERROR);

        inflateEnd(&stream);
        return ret == Z_STREAM_END;
    }

    const char* WebsocketParsingUtils::scanTo(const char* p, const char* end, const char* pattern, size_t len)
    {
        const char* found = static_cast<const char*>(memmem(p, end - p, pattern, len));
        return found ? found + len : nullptr;
    }

    uint64_t WebsocketParsingUtils::parseUint64Fast(const char*& p, const char* end)
    {
        uint64_t val = 0;
        while (p < end && *p >= '0' && *p <= '9')
        {
            val = val * 10 + (*p - '0');
            p++;
        }
        return val;
    }
}
