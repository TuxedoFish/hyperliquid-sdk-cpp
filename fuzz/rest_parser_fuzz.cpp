#include "hyperliquid/rest/RestApiMessageParser.h"

#include <cstddef>
#include <cstdint>
#include <exception>
#include <string>

// libFuzzer harness for RestApiMessageParser.
//
// Rather than fuzzing RestApiMessageParser::parse() (which just dispatches on
// RestEndpointType to one of the public parseXxx() methods below and forwards
// the result to a listener callback), this harness calls every public
// parseXxx() method directly. That gives full coverage of the actual
// simdjson ondemand parsing logic - the untrusted-input boundary the issue is
// concerned with - without needing 48 separate fuzz binaries. The first byte
// of the input selects which parser to exercise; the remaining bytes are fed
// to it verbatim as the "response body".
//
// Exceptions (simdjson_error on malformed JSON, in particular) are expected,
// normal behavior for hostile/malformed input and are swallowed here so
// libFuzzer's crash detection stays focused on real robustness bugs: crashes,
// ASan/UBSan violations, and hangs.
//
// This harness is seeded with real captured response payloads. The plain,
// valid JSON lives in examples/json/rest/<parserName>/*.json (reusable for
// anything else - docs, mocking, OpenAPI examples); run
// fuzz/generate_corpus.py to turn that into an actual corpus directory for
// this binary, which prepends each file's selector byte (its index into
// kParsers[] below) - that prefix is what makes a corpus file valid input
// here, not just the JSON body.

namespace
{
    using hyperliquid::RestApiMessageParser;

    using ParseFn = void (*)(RestApiMessageParser&, const std::string&);

    template <auto Method>
    void invoke(RestApiMessageParser& parser, const std::string& message)
    {
        (parser.*Method)(message);
    }

    constexpr ParseFn kParsers[] = {
        invoke<&RestApiMessageParser::parseSpotMeta>,
        invoke<&RestApiMessageParser::parseMeta>,
        invoke<&RestApiMessageParser::parseOutcomeMeta>,
        invoke<&RestApiMessageParser::parseSettledOutcome>,
        invoke<&RestApiMessageParser::parsePerpDexs>,
        invoke<&RestApiMessageParser::parsePerpsAtOpenInterestCap>,
        invoke<&RestApiMessageParser::parsePredictedFundings>,
        invoke<&RestApiMessageParser::parsePerpAnnotation>,
        invoke<&RestApiMessageParser::parsePerpCategories>,
        invoke<&RestApiMessageParser::parsePerpConciseAnnotations>,
        invoke<&RestApiMessageParser::parseAllPerpMetas>,
        invoke<&RestApiMessageParser::parsePerpDexLimits>,
        invoke<&RestApiMessageParser::parsePerpDexStatus>,
        invoke<&RestApiMessageParser::parsePerpDeployAuctionStatus>,
        invoke<&RestApiMessageParser::parseL2Book>,
        invoke<&RestApiMessageParser::parseCandleSnapshot>,
        invoke<&RestApiMessageParser::parseAllMids>,
        invoke<&RestApiMessageParser::parseOpenOrders>,
        invoke<&RestApiMessageParser::parseOrderStatus>,
        invoke<&RestApiMessageParser::parseUserFills>,
        invoke<&RestApiMessageParser::parseUserFillsByTime>,
        invoke<&RestApiMessageParser::parseClearinghouseState>,
        invoke<&RestApiMessageParser::parseUserRateLimit>,
        invoke<&RestApiMessageParser::parseMetaAndAssetCtxs>,
        invoke<&RestApiMessageParser::parseSpotMetaAndAssetCtxs>,
        invoke<&RestApiMessageParser::parseSpotClearinghouseState>,
        invoke<&RestApiMessageParser::parseFrontendOpenOrders>,
        invoke<&RestApiMessageParser::parseHistoricalOrders>,
        invoke<&RestApiMessageParser::parseUserTwapSliceFills>,
        invoke<&RestApiMessageParser::parseSubAccounts>,
        invoke<&RestApiMessageParser::parseUserFees>,
        invoke<&RestApiMessageParser::parseMaxBuilderFee>,
        invoke<&RestApiMessageParser::parseApprovedBuilders>,
        invoke<&RestApiMessageParser::parseVaultDetails>,
        invoke<&RestApiMessageParser::parseUserVaultEquities>,
        invoke<&RestApiMessageParser::parsePortfolio>,
        invoke<&RestApiMessageParser::parseReferral>,
        invoke<&RestApiMessageParser::parseUserRole>,
        invoke<&RestApiMessageParser::parseBorrowLendUserState>,
        invoke<&RestApiMessageParser::parseBorrowLendReserveState>,
        invoke<&RestApiMessageParser::parseAllBorrowLendReserveStates>,
        invoke<&RestApiMessageParser::parsePlaceOrder>,
        invoke<&RestApiMessageParser::parseCancelOrder>,
        invoke<&RestApiMessageParser::parseModifyOrder>,
        invoke<&RestApiMessageParser::parseSimpleResponse>,
        invoke<&RestApiMessageParser::parseTwapOrder>,
        invoke<&RestApiMessageParser::parseTwapCancel>,
        invoke<&RestApiMessageParser::parseDelegations>,
        invoke<&RestApiMessageParser::parseDelegatorSummary>,
        invoke<&RestApiMessageParser::parseDelegatorHistory>,
        invoke<&RestApiMessageParser::parseDelegatorRewards>,
        invoke<&RestApiMessageParser::parseSpotDeployState>,
        invoke<&RestApiMessageParser::parseSpotPairDeployAuctionStatus>,
    };

    constexpr size_t kParserCount = sizeof(kParsers) / sizeof(kParsers[0]);

    RestApiMessageParser& sharedParser()
    {
        // A single long-lived instance, reused across every fuzz iteration -
        // this matches how the SDK actually uses the parser (one instance per
        // connection, fed a stream of messages) and lets the fuzzer surface
        // any state-corruption bugs from a malformed message poisoning the
        // parser's internal simdjson::ondemand::parser/padded_string state
        // for the next one.
        static RestApiMessageParser parser;
        return parser;
    }
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < 1) return 0;

    const uint8_t selector = data[0];
    const std::string message(reinterpret_cast<const char*>(data + 1), size - 1);

    try
    {
        kParsers[selector % kParserCount](sharedParser(), message);
    }
    catch (const std::exception&)
    {
    }
    catch (...)
    {
    }

    return 0;
}
