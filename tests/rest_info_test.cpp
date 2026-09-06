#include <gtest/gtest.h>

#include "messages/InfoRequestBuilder.h"
#include "hyperliquid/rest/RestApiMessageParser.h"

#include <nlohmann/json.hpp>

using namespace hyperliquid;

// --- Request building ---

TEST(InfoRequestBuilderTest, L2BookMinimal)
{
    auto body = InfoRequestBuilder::l2Book("BTC");
    EXPECT_EQ(body["type"], "l2Book");
    EXPECT_EQ(body["coin"], "BTC");
    EXPECT_FALSE(body.contains("nSigFigs"));
    EXPECT_FALSE(body.contains("mantissa"));
}

TEST(InfoRequestBuilderTest, L2BookWithSigFigsAndMantissa)
{
    auto body = InfoRequestBuilder::l2Book("BTC", 5, 2);
    EXPECT_EQ(body["type"], "l2Book");
    EXPECT_EQ(body["coin"], "BTC");
    EXPECT_EQ(body["nSigFigs"], 5);
    EXPECT_EQ(body["mantissa"], 2);
}

TEST(InfoRequestBuilderTest, CandleSnapshot)
{
    auto body = InfoRequestBuilder::candleSnapshot("ETH", "15m", 1681222254710ULL, 1681223254710ULL);
    EXPECT_EQ(body["type"], "candleSnapshot");
    ASSERT_TRUE(body.contains("req"));
    EXPECT_EQ(body["req"]["coin"], "ETH");
    EXPECT_EQ(body["req"]["interval"], "15m");
    EXPECT_EQ(body["req"]["startTime"], 1681222254710ULL);
    EXPECT_EQ(body["req"]["endTime"], 1681223254710ULL);
}

TEST(InfoRequestBuilderTest, AllMidsNoDex)
{
    auto body = InfoRequestBuilder::allMids();
    EXPECT_EQ(body["type"], "allMids");
    EXPECT_FALSE(body.contains("dex"));
}

TEST(InfoRequestBuilderTest, AllMidsWithDex)
{
    auto body = InfoRequestBuilder::allMids("test-dex");
    EXPECT_EQ(body["type"], "allMids");
    EXPECT_EQ(body["dex"], "test-dex");
}

TEST(InfoRequestBuilderTest, OpenOrders)
{
    auto body = InfoRequestBuilder::openOrders("0xabc");
    EXPECT_EQ(body["type"], "openOrders");
    EXPECT_EQ(body["user"], "0xabc");
    EXPECT_FALSE(body.contains("dex"));
}

TEST(InfoRequestBuilderTest, OrderStatusByOid)
{
    auto body = InfoRequestBuilder::orderStatus("0xabc", OrderId{uint64_t{91490942}});
    EXPECT_EQ(body["type"], "orderStatus");
    EXPECT_EQ(body["user"], "0xabc");
    EXPECT_EQ(body["oid"], 91490942ULL);
    EXPECT_TRUE(body["oid"].is_number());
}

TEST(InfoRequestBuilderTest, OrderStatusByCloid)
{
    std::string cloid = "0x00000000000000000000000000000001";
    auto body = InfoRequestBuilder::orderStatus("0xabc", OrderId{cloid});
    EXPECT_EQ(body["type"], "orderStatus");
    EXPECT_EQ(body["oid"], cloid);
    EXPECT_TRUE(body["oid"].is_string());
}

TEST(InfoRequestBuilderTest, UserFillsDefaults)
{
    auto body = InfoRequestBuilder::userFills("0xabc");
    EXPECT_EQ(body["type"], "userFills");
    EXPECT_EQ(body["user"], "0xabc");
    EXPECT_FALSE(body.contains("aggregateByTime"));
    EXPECT_FALSE(body.contains("dex"));
}

TEST(InfoRequestBuilderTest, UserFillsAggregated)
{
    auto body = InfoRequestBuilder::userFills("0xabc", true);
    EXPECT_EQ(body["aggregateByTime"], true);
}

TEST(InfoRequestBuilderTest, UserFillsByTime)
{
    auto body = InfoRequestBuilder::userFillsByTime("0xabc", 1681222254710ULL, 1681223254710ULL, true);
    EXPECT_EQ(body["type"], "userFillsByTime");
    EXPECT_EQ(body["user"], "0xabc");
    EXPECT_EQ(body["startTime"], 1681222254710ULL);
    EXPECT_EQ(body["endTime"], 1681223254710ULL);
    EXPECT_EQ(body["aggregateByTime"], true);
}

TEST(InfoRequestBuilderTest, UserFillsByTimeStartOnly)
{
    auto body = InfoRequestBuilder::userFillsByTime("0xabc", 1681222254710ULL);
    EXPECT_EQ(body["startTime"], 1681222254710ULL);
    EXPECT_FALSE(body.contains("endTime"));
    EXPECT_FALSE(body.contains("aggregateByTime"));
}

TEST(InfoRequestBuilderTest, ClearinghouseState)
{
    auto body = InfoRequestBuilder::clearinghouseState("0xabc");
    EXPECT_EQ(body["type"], "clearinghouseState");
    EXPECT_EQ(body["user"], "0xabc");
    EXPECT_FALSE(body.contains("dex"));
}

// --- Response parsing ---

TEST(RestApiMessageParserInfoTest, ParseL2Book)
{
    std::string message = R"({
        "coin": "BTC",
        "time": 1754450974231,
        "levels": [
            [{"px": "113377.0", "sz": "7.6699", "n": 17}, {"px": "113370.0", "sz": "1.0", "n": 2}],
            [{"px": "113400.0", "sz": "2.5", "n": 3}]
        ]
    })";

    RestApiMessageParser parser;
    auto book = parser.parseL2Book(message);

    EXPECT_EQ(book.coin, "BTC");
    EXPECT_EQ(book.time, 1754450974231ULL);
    ASSERT_EQ(book.bids.size(), 2u);
    EXPECT_EQ(book.bids[0].px, "113377.0");
    EXPECT_EQ(book.bids[0].sz, "7.6699");
    EXPECT_EQ(book.bids[0].n, 17);
    ASSERT_EQ(book.asks.size(), 1u);
    EXPECT_EQ(book.asks[0].px, "113400.0");
}

TEST(RestApiMessageParserInfoTest, ParseCandleSnapshot)
{
    std::string message = R"([
        {"T": 1681924499999, "c": "29258.0", "h": "29309.0", "i": "15m", "l": "29250.0",
         "n": 12, "o": "29250.0", "s": "BTC", "t": 1681923600000, "v": "10.5"}
    ])";

    RestApiMessageParser parser;
    auto snapshot = parser.parseCandleSnapshot(message);

    ASSERT_EQ(snapshot.candles.size(), 1u);
    const auto& c = snapshot.candles[0];
    EXPECT_EQ(c.coin, "BTC");
    EXPECT_EQ(c.interval, "15m");
    EXPECT_EQ(c.openTime, 1681923600000ULL);
    EXPECT_EQ(c.closeTime, 1681924499999ULL);
    EXPECT_DOUBLE_EQ(c.open, 29250.0);
    EXPECT_DOUBLE_EQ(c.close, 29258.0);
    EXPECT_DOUBLE_EQ(c.high, 29309.0);
    EXPECT_DOUBLE_EQ(c.low, 29250.0);
    EXPECT_DOUBLE_EQ(c.volume, 10.5);
    EXPECT_EQ(c.numTrades, 12);
}

TEST(RestApiMessageParserInfoTest, ParseAllMids)
{
    std::string message = R"({"APE": "4.33245", "ARB": "1.21695"})";

    RestApiMessageParser parser;
    auto mids = parser.parseAllMids(message);

    ASSERT_EQ(mids.mids.size(), 2u);
    EXPECT_EQ(mids.mids[0].coin, "APE");
    EXPECT_DOUBLE_EQ(mids.mids[0].mid, 4.33245);
    EXPECT_EQ(mids.mids[1].coin, "ARB");
    EXPECT_DOUBLE_EQ(mids.mids[1].mid, 1.21695);
}

TEST(RestApiMessageParserInfoTest, ParseOpenOrders)
{
    std::string message = R"([
        {"coin": "ETH", "side": "B", "limitPx": "1800.0", "sz": "0.5", "oid": 123,
         "timestamp": 1724361546645, "origSz": "1.0", "cloid": null}
    ])";

    RestApiMessageParser parser;
    auto orders = parser.parseOpenOrders(message);

    ASSERT_EQ(orders.orders.size(), 1u);
    const auto& o = orders.orders[0];
    EXPECT_EQ(o.coin, "ETH");
    EXPECT_EQ(o.side, 'B');
    EXPECT_DOUBLE_EQ(o.limitPx, 1800.0);
    EXPECT_DOUBLE_EQ(o.sz, 0.5);
    EXPECT_EQ(o.oid, 123u);
    EXPECT_EQ(o.timestamp, 1724361546645ULL);
    EXPECT_DOUBLE_EQ(o.origSz, 1.0);
    EXPECT_TRUE(o.cloid.empty());
}

TEST(RestApiMessageParserInfoTest, ParseOrderStatusFound)
{
    std::string message = R"({
        "status": "order",
        "order": {
            "order": {
                "coin": "ETH",
                "side": "A",
                "limitPx": "2412.7",
                "sz": "0.0",
                "oid": 1,
                "timestamp": 1724361546645,
                "origSz": "0.0076",
                "cloid": null
            },
            "status": "filled",
            "statusTimestamp": 1724361546645
        }
    })";

    RestApiMessageParser parser;
    auto result = parser.parseOrderStatus(message);

    EXPECT_EQ(result.status, "order");
    ASSERT_TRUE(result.order.has_value());
    EXPECT_EQ(result.order->order.coin, "ETH");
    EXPECT_EQ(result.order->order.oid, 1u);
    EXPECT_EQ(result.order->status, OrderStatus::Filled);
    EXPECT_EQ(result.order->statusTimestamp, 1724361546645ULL);
}

TEST(RestApiMessageParserInfoTest, ParseOrderStatusUnknown)
{
    std::string message = R"({"status": "unknownOid"})";

    RestApiMessageParser parser;
    auto result = parser.parseOrderStatus(message);

    EXPECT_EQ(result.status, "unknownOid");
    EXPECT_FALSE(result.order.has_value());
}

TEST(RestApiMessageParserInfoTest, ParseUserFills)
{
    std::string message = R"([
        {"coin": "ETH", "px": "1800.0", "sz": "0.1", "side": "B", "time": 1724361546645,
         "startPosition": "0.0", "dir": "Open Long", "closedPnl": "0.0", "hash": "0xabc",
         "oid": 55, "crossed": false, "fee": "0.01", "tid": 999, "feeToken": "USDC"}
    ])";

    RestApiMessageParser parser;
    auto fills = parser.parseUserFills(message);

    ASSERT_EQ(fills.fills.size(), 1u);
    const auto& f = fills.fills[0];
    EXPECT_EQ(f.coin, "ETH");
    EXPECT_DOUBLE_EQ(f.px, 1800.0);
    EXPECT_DOUBLE_EQ(f.sz, 0.1);
    EXPECT_EQ(f.side, 'B');
    EXPECT_EQ(f.oid, 55u);
    EXPECT_EQ(f.tid, 999u);
    EXPECT_EQ(f.feeToken, "USDC");
}

TEST(RestApiMessageParserInfoTest, ParseUserFillsByTime)
{
    // parseUserFillsByTime delegates straight to parseUserFills (same response shape) - reuses
    // the exact payload from ParseUserFills above.
    std::string message = R"([
        {"coin": "ETH", "px": "1800.0", "sz": "0.1", "side": "B", "time": 1724361546645,
         "startPosition": "0.0", "dir": "Open Long", "closedPnl": "0.0", "hash": "0xabc",
         "oid": 55, "crossed": false, "fee": "0.01", "tid": 999, "feeToken": "USDC"}
    ])";

    RestApiMessageParser parser;
    auto fills = parser.parseUserFillsByTime(message);

    ASSERT_EQ(fills.fills.size(), 1u);
    EXPECT_EQ(fills.fills[0].coin, "ETH");
    EXPECT_EQ(fills.fills[0].oid, 55u);
}

TEST(RestApiMessageParserInfoTest, ParseClearinghouseState)
{
    std::string message = R"({
        "assetPositions": [
            {
                "position": {
                    "coin": "ETH",
                    "entryPx": "2986.3",
                    "positionValue": "100.02765",
                    "returnOnEquity": "-0.0026789",
                    "szi": "0.0335",
                    "unrealizedPnl": "-0.0134",
                    "liquidationPx": "2866.26936529"
                },
                "type": "oneWay"
            }
        ],
        "marginSummary": {"accountValue": "13109.482328", "totalMarginUsed": "4.967826",
                           "totalNtlPos": "100.02765", "totalRawUsd": "13009.454678"},
        "crossMarginSummary": {"accountValue": "13109.482328", "totalMarginUsed": "4.967826",
                                "totalNtlPos": "100.02765", "totalRawUsd": "13009.454678"},
        "crossMaintenanceMarginUsed": "1.234",
        "time": 1708622398623,
        "withdrawable": "13104.514502"
    })";

    RestApiMessageParser parser;
    auto state = parser.parseClearinghouseState(message);

    EXPECT_DOUBLE_EQ(state.marginSummary.accountValue, 13109.482328);
    EXPECT_DOUBLE_EQ(state.crossMarginSummary.accountValue, 13109.482328);
    EXPECT_DOUBLE_EQ(state.crossMaintenanceMarginUsed, 1.234);
    EXPECT_DOUBLE_EQ(state.withdrawable, 13104.514502);
    EXPECT_EQ(state.time, 1708622398623ULL);
    ASSERT_EQ(state.assetPositions.size(), 1u);
    EXPECT_EQ(state.assetPositions[0].coin, "ETH");
    EXPECT_DOUBLE_EQ(state.assetPositions[0].szi, 0.0335);
    EXPECT_TRUE(state.assetPositions[0].hasLiquidationPx);
    EXPECT_DOUBLE_EQ(state.assetPositions[0].liquidationPx, 2866.26936529);
}

// --- Metadata endpoints: request building ---

TEST(InfoRequestBuilderTest, MetaAndAssetCtxsNoDex)
{
    auto body = InfoRequestBuilder::metaAndAssetCtxs();
    EXPECT_EQ(body["type"], "metaAndAssetCtxs");
    EXPECT_FALSE(body.contains("dex"));
}

TEST(InfoRequestBuilderTest, MetaAndAssetCtxsWithDex)
{
    auto body = InfoRequestBuilder::metaAndAssetCtxs("test-dex");
    EXPECT_EQ(body["dex"], "test-dex");
}

TEST(InfoRequestBuilderTest, SpotMetaAndAssetCtxs)
{
    auto body = InfoRequestBuilder::spotMetaAndAssetCtxs();
    EXPECT_EQ(body["type"], "spotMetaAndAssetCtxs");
}

TEST(InfoRequestBuilderTest, SpotClearinghouseState)
{
    auto body = InfoRequestBuilder::spotClearinghouseState("0xabc");
    EXPECT_EQ(body["type"], "spotClearinghouseState");
    EXPECT_EQ(body["user"], "0xabc");
}

TEST(InfoRequestBuilderTest, FrontendOpenOrders)
{
    auto body = InfoRequestBuilder::frontendOpenOrders("0xabc");
    EXPECT_EQ(body["type"], "frontendOpenOrders");
    EXPECT_EQ(body["user"], "0xabc");
}

TEST(InfoRequestBuilderTest, HistoricalOrders)
{
    auto body = InfoRequestBuilder::historicalOrders("0xabc");
    EXPECT_EQ(body["type"], "historicalOrders");
    EXPECT_EQ(body["user"], "0xabc");
}

TEST(InfoRequestBuilderTest, UserTwapSliceFills)
{
    auto body = InfoRequestBuilder::userTwapSliceFills("0xabc");
    EXPECT_EQ(body["type"], "userTwapSliceFills");
    EXPECT_EQ(body["user"], "0xabc");
}

TEST(InfoRequestBuilderTest, SubAccounts)
{
    auto body = InfoRequestBuilder::subAccounts("0xabc");
    EXPECT_EQ(body["type"], "subAccounts");
    EXPECT_EQ(body["user"], "0xabc");
}

TEST(InfoRequestBuilderTest, UserFees)
{
    auto body = InfoRequestBuilder::userFees("0xabc");
    EXPECT_EQ(body["type"], "userFees");
    EXPECT_EQ(body["user"], "0xabc");
}

TEST(InfoRequestBuilderTest, MaxBuilderFee)
{
    auto body = InfoRequestBuilder::maxBuilderFee("0xabc", "0xbuilder");
    EXPECT_EQ(body["type"], "maxBuilderFee");
    EXPECT_EQ(body["user"], "0xabc");
    EXPECT_EQ(body["builder"], "0xbuilder");
}

TEST(InfoRequestBuilderTest, ApprovedBuilders)
{
    auto body = InfoRequestBuilder::approvedBuilders("0xabc");
    EXPECT_EQ(body["type"], "approvedBuilders");
    EXPECT_EQ(body["user"], "0xabc");
}

// --- Metadata endpoints: response parsing ---

TEST(RestApiMessageParserInfoTest, ParseSpotMeta)
{
    // Real testnet response, trimmed to 3 tokens covering the evmContract-present/fullName-null,
    // evmContract-present/fullName-present, and evmContract-null/non-canonical variants. The
    // parser only reads "tokens" - the real response's "universe" (trading-pair) array isn't
    // currently parsed by parseSpotMeta.
    std::string message = R"({"tokens": [
        {"name": "USDC", "szDecimals": 8, "weiDecimals": 8, "index": 0, "tokenId": "0xeb62eee3685fc4c43992febcd9e75443", "isCanonical": true, "evmContract": {"address": "0x0b80659a4076e9e93c7dbe0f10675a16a3e5c206", "evm_extra_wei_decimals": -2}, "fullName": null},
        {"name": "PURR", "szDecimals": 0, "weiDecimals": 5, "index": 1, "tokenId": "0xc4bf3f870c0e9465323c0b6ed28096c2", "isCanonical": true, "evmContract": {"address": "0xa9056c15938f9aff34cd497c722ce33db0c2fd57", "evm_extra_wei_decimals": 13}, "fullName": "Hypurr"},
        {"name": "TEST", "szDecimals": 1, "weiDecimals": 8, "index": 2, "tokenId": "0x98b101daf4ff26697646131261c100bf", "isCanonical": false, "evmContract": null, "fullName": null}
    ]})";

    RestApiMessageParser parser;
    auto response = parser.parseSpotMeta(message);

    ASSERT_EQ(response.tokens.size(), 3u);
    EXPECT_EQ(response.tokens[0].name, "USDC");
    ASSERT_TRUE(response.tokens[0].evmContract.has_value());
    EXPECT_EQ(response.tokens[0].evmContract->address, "0x0b80659a4076e9e93c7dbe0f10675a16a3e5c206");
    EXPECT_FALSE(response.tokens[0].fullName.has_value());
    EXPECT_EQ(response.tokens[1].name, "PURR");
    ASSERT_TRUE(response.tokens[1].fullName.has_value());
    EXPECT_EQ(*response.tokens[1].fullName, "Hypurr");
    EXPECT_EQ(response.tokens[2].name, "TEST");
    EXPECT_FALSE(response.tokens[2].isCanonical);
    EXPECT_FALSE(response.tokens[2].evmContract.has_value());
}

TEST(RestApiMessageParserInfoTest, ParseOutcomeMeta)
{
    // Real testnet response, trimmed to 2 outcomes - one with an empty description, one with
    // the pipe-delimited "class:X|underlying:Y|..." format parsed by parseOutcomeDescription.
    std::string message = R"({"outcomes": [
        {"outcome": 10217, "name": "Fallback", "description": "", "sideSpecs": [{"name": "Yes"}, {"name": "No"}], "quoteToken": "USDC"},
        {"outcome": 17495, "name": "Recurring", "description": "class:priceBinary|underlying:BTC|expiry:20260907-0300|targetPrice:80115|period:1d", "sideSpecs": [{"name": "Yes"}, {"name": "No"}], "quoteToken": "USDC"}
    ]})";

    RestApiMessageParser parser;
    auto response = parser.parseOutcomeMeta(message);

    ASSERT_EQ(response.outcomes.size(), 2u);
    EXPECT_EQ(response.outcomes[0].outcome, 10217);
    EXPECT_EQ(response.outcomes[0].name, "Fallback");
    EXPECT_TRUE(response.outcomes[0].descriptionRaw.empty());
    EXPECT_EQ(response.outcomes[1].outcome, 17495);
    EXPECT_EQ(response.outcomes[1].description.outcomeClass, "priceBinary");
    EXPECT_EQ(response.outcomes[1].description.underlying, "BTC");
    EXPECT_EQ(response.outcomes[1].description.targetPrice, "80115");
}

TEST(RestApiMessageParserInfoTest, ParsePerpDexs)
{
    // Real testnet response, trimmed to the leading null (always the main dex - skipped by the
    // parser) plus 2 real HIP-3 dexes covering both the empty and populated
    // assetToStreamingOiCap/assetToFundingMultiplier variants.
    std::string message =
        R"([null,{"name":"test","fullName":"test dex","deployer":"0x5e89b26d8d66da9888c835c9bfcc2aa51813e152","oracleUpdater":null,"feeRecipient":null,"assetToStreamingOiCap":[],"assetToFundingMultiplier":[]},{"name":"felix","fullName":"felix","deployer":"0x3a4ca3a93fc224c0a073d087c19ba8f0f04c7f00","oracleUpdater":null,"feeRecipient":"0x3a4ca3a93fc224c0a073d087c19ba8f0f04c7f00","assetToStreamingOiCap":[["felix:CRCL","2500000.0"]],"assetToFundingMultiplier":[["felix:CRCL","1.0"]]}])";

    RestApiMessageParser parser;
    auto response = parser.parsePerpDexs(message);

    ASSERT_EQ(response.dexes.size(), 2u);
    EXPECT_EQ(response.dexes[0].name, "test");
    EXPECT_FALSE(response.dexes[0].feeRecipient.has_value());
    EXPECT_TRUE(response.dexes[0].assetToStreamingOiCap.empty());
    EXPECT_EQ(response.dexes[1].name, "felix");
    ASSERT_TRUE(response.dexes[1].feeRecipient.has_value());
    EXPECT_EQ(*response.dexes[1].feeRecipient, "0x3a4ca3a93fc224c0a073d087c19ba8f0f04c7f00");
    ASSERT_EQ(response.dexes[1].assetToStreamingOiCap.size(), 1u);
    EXPECT_EQ(response.dexes[1].assetToStreamingOiCap[0].first, "felix:CRCL");
}

TEST(RestApiMessageParserInfoTest, ParseMetaAndAssetCtxs)
{
    std::string message = R"([
        {"universe": [{"name": "BTC", "szDecimals": 5, "maxLeverage": 50}]},
        [{"dayNtlVlm": "1000.5", "prevDayPx": "60000.0", "markPx": "61000.0",
          "midPx": "61000.5", "funding": "0.0001", "openInterest": "500.0", "oraclePx": "61000.2"}]
    ])";

    RestApiMessageParser parser;
    auto response = parser.parseMetaAndAssetCtxs(message);

    ASSERT_EQ(response.meta.universe.size(), 1u);
    EXPECT_EQ(response.meta.universe[0].name, "BTC");
    ASSERT_EQ(response.assetCtxs.size(), 1u);
    EXPECT_EQ(response.assetCtxs[0].coin, "BTC");
    EXPECT_DOUBLE_EQ(response.assetCtxs[0].markPx, 61000.0);
    EXPECT_TRUE(response.assetCtxs[0].hasMidPx);
    EXPECT_DOUBLE_EQ(response.assetCtxs[0].midPx, 61000.5);
}

TEST(RestApiMessageParserInfoTest, ParseSpotMetaAndAssetCtxs)
{
    std::string message = R"([
        {
            "tokens": [{"name": "USDC", "szDecimals": 8, "weiDecimals": 8, "index": 0, "tokenId": "0x1"}],
            "universe": [{"name": "PURR/USDC", "tokens": [1, 0], "index": 0, "isCanonical": true}]
        },
        [{"dayNtlVlm": "500.0", "prevDayPx": "0.5", "markPx": "0.51", "circulatingSupply": "1000000.0"}]
    ])";

    RestApiMessageParser parser;
    auto response = parser.parseSpotMetaAndAssetCtxs(message);

    ASSERT_EQ(response.meta.tokens.size(), 1u);
    EXPECT_EQ(response.meta.tokens[0].name, "USDC");
    ASSERT_EQ(response.meta.universe.size(), 1u);
    EXPECT_EQ(response.meta.universe[0].name, "PURR/USDC");
    EXPECT_TRUE(response.meta.universe[0].isCanonical);
    ASSERT_EQ(response.assetCtxs.size(), 1u);
    EXPECT_EQ(response.assetCtxs[0].coin, "PURR/USDC");
    EXPECT_DOUBLE_EQ(response.assetCtxs[0].circulatingSupply, 1000000.0);
}

TEST(RestApiMessageParserInfoTest, ParseSpotClearinghouseState)
{
    std::string message = R"({
        "balances": [
            {"coin": "USDC", "token": 0, "hold": "0.0", "total": "508.05193", "entryNtl": "500.0"}
        ]
    })";

    RestApiMessageParser parser;
    auto response = parser.parseSpotClearinghouseState(message);

    ASSERT_EQ(response.balances.size(), 1u);
    EXPECT_EQ(response.balances[0].coin, "USDC");
    EXPECT_DOUBLE_EQ(response.balances[0].total, 508.05193);
}

TEST(RestApiMessageParserInfoTest, ParseFrontendOpenOrders)
{
    std::string message = R"([{
        "coin": "ETH", "side": "B", "limitPx": "1670.1", "sz": "0.5", "oid": 123, "timestamp": 1000,
        "origSz": "0.5", "isPositionTpsl": false, "isTrigger": false, "triggerPx": "0.0",
        "triggerCondition": "N/A", "reduceOnly": false, "orderType": "Limit", "tif": "Gtc"
    }])";

    RestApiMessageParser parser;
    auto response = parser.parseFrontendOpenOrders(message);

    ASSERT_EQ(response.orders.size(), 1u);
    EXPECT_EQ(response.orders[0].coin, "ETH");
    EXPECT_EQ(response.orders[0].orderType, FrontendOrderType::Limit);
    ASSERT_TRUE(response.orders[0].tif.has_value());
    EXPECT_EQ(*response.orders[0].tif, OrderTif::Gtc);
}

TEST(RestApiMessageParserInfoTest, ParseHistoricalOrders)
{
    std::string message = R"([{
        "order": {
            "coin": "ETH", "side": "B", "limitPx": "1670.1", "sz": "0.5", "oid": 123, "timestamp": 1000,
            "origSz": "0.5", "isPositionTpsl": false, "isTrigger": false, "triggerPx": "0.0",
            "triggerCondition": "N/A", "reduceOnly": false, "orderType": "Limit"
        },
        "status": "filled",
        "statusTimestamp": 1788531646497
    }])";

    RestApiMessageParser parser;
    auto response = parser.parseHistoricalOrders(message);

    ASSERT_EQ(response.orders.size(), 1u);
    EXPECT_EQ(response.orders[0].order.coin, "ETH");
    EXPECT_EQ(response.orders[0].status, OrderStatus::Filled);
    EXPECT_EQ(response.orders[0].statusTimestamp, 1788531646497ULL);
}

TEST(RestApiMessageParserInfoTest, ParseUserTwapSliceFills)
{
    std::string message = R"([{
        "fill": {
            "coin": "ETH", "px": "1670.1", "sz": "0.5", "side": "B", "time": 1000,
            "startPosition": "0.0", "dir": "Open Long", "closedPnl": "0.0", "hash": "0xabc",
            "oid": 1, "crossed": true, "fee": "0.1", "tid": 1, "feeToken": "USDC"
        },
        "twapId": 42
    }])";

    RestApiMessageParser parser;
    auto response = parser.parseUserTwapSliceFills(message);

    ASSERT_EQ(response.fills.size(), 1u);
    EXPECT_EQ(response.fills[0].fill.coin, "ETH");
    EXPECT_EQ(response.fills[0].twapId, 42);
}

TEST(RestApiMessageParserInfoTest, ParseSubAccountsEmptyList)
{
    RestApiMessageParser parser;
    auto response = parser.parseSubAccounts("[]");
    EXPECT_TRUE(response.subAccounts.empty());
}

// Regression test: the API returns a bare `null` (not `[]`) when the user has no
// sub-accounts, which previously threw a parse error (INCORRECT_TYPE) instead of
// being treated as "no sub-accounts".
TEST(RestApiMessageParserInfoTest, ParseSubAccountsNullResponse)
{
    RestApiMessageParser parser;
    auto response = parser.parseSubAccounts("null");
    EXPECT_TRUE(response.subAccounts.empty());
}

TEST(RestApiMessageParserInfoTest, ParseSubAccountsPopulated)
{
    std::string message = R"([{
        "name": "sub1",
        "subAccountUser": "0xsub1",
        "master": "0xmaster",
        "clearinghouseState": {
            "assetPositions": [], "marginSummary": {"accountValue": "0.0", "totalMarginUsed": "0.0",
            "totalNtlPos": "0.0", "totalRawUsd": "0.0"},
            "crossMarginSummary": {"accountValue": "0.0", "totalMarginUsed": "0.0",
            "totalNtlPos": "0.0", "totalRawUsd": "0.0"},
            "crossMaintenanceMarginUsed": "0.0", "time": 1000, "withdrawable": "0.0"
        },
        "spotState": {"balances": []}
    }])";

    RestApiMessageParser parser;
    auto response = parser.parseSubAccounts(message);

    ASSERT_EQ(response.subAccounts.size(), 1u);
    EXPECT_EQ(response.subAccounts[0].name, "sub1");
    EXPECT_EQ(response.subAccounts[0].subAccountUser, "0xsub1");
}

// Regression test: the API nests VIP/MM fee tiers under feeSchedule.tiers.{vip,mm}
// (not feeSchedule.vipTiers/.mmTiers directly), and the trial-reward field is named
// feeTrialEscrow (not feeTrialReward) - both previously threw a parse error
// (NO_SUCH_FIELD) against a real response.
TEST(RestApiMessageParserInfoTest, ParseUserFees)
{
    std::string message = R"({
        "dailyUserVlm": [{"date": "2026-09-04", "userCross": "73.82", "userAdd": "0.0", "exchange": "4574204.83"}],
        "feeSchedule": {
            "cross": "0.00045", "add": "0.00015", "spotCross": "0.0007", "spotAdd": "0.0004",
            "tiers": {
                "vip": [{"ntlCutoff": "5000000.0", "cross": "0.0004", "add": "0.00012", "spotCross": "0.0006", "spotAdd": "0.0003"}],
                "mm": [{"makerFractionCutoff": "0.005", "add": "-0.00001"}]
            },
            "referralDiscount": "0.04",
            "stakingDiscountTiers": [{"bpsOfMaxSupply": "0.0", "discount": "0.0"}]
        },
        "userCrossRate": "0.00045", "userAddRate": "0.00015",
        "userSpotCrossRate": "0.0007", "userSpotAddRate": "0.0004",
        "activeReferralDiscount": "0.0", "feeTrialEscrow": "0.0",
        "nextTrialAvailableTimestamp": null, "stakingLink": null,
        "activeStakingDiscount": {"bpsOfMaxSupply": "0.0", "discount": "0.0"}
    })";

    RestApiMessageParser parser;
    auto response = parser.parseUserFees(message);

    ASSERT_EQ(response.dailyUserVlm.size(), 1u);
    EXPECT_DOUBLE_EQ(response.dailyUserVlm[0].userCross, 73.82);
    ASSERT_EQ(response.feeSchedule.vipTiers.size(), 1u);
    EXPECT_DOUBLE_EQ(response.feeSchedule.vipTiers[0].ntlCutoff, 5000000.0);
    ASSERT_EQ(response.feeSchedule.mmTiers.size(), 1u);
    EXPECT_DOUBLE_EQ(response.feeSchedule.mmTiers[0].add, -0.00001);
    EXPECT_DOUBLE_EQ(response.userCrossRate, 0.00045);
    EXPECT_DOUBLE_EQ(response.feeTrialEscrow, 0.0);
    EXPECT_FALSE(response.nextTrialAvailableTimestamp.has_value());
    EXPECT_FALSE(response.stakingLink.has_value());
    ASSERT_TRUE(response.activeStakingDiscount.has_value());
}

TEST(RestApiMessageParserInfoTest, ParseMaxBuilderFee)
{
    RestApiMessageParser parser;
    auto response = parser.parseMaxBuilderFee("100");
    EXPECT_EQ(response.maxFeeRateTenthsBps, 100);
}

TEST(RestApiMessageParserInfoTest, ParseApprovedBuilders)
{
    std::string message = R"(["0xbuilder1", "0xbuilder2"])";

    RestApiMessageParser parser;
    auto response = parser.parseApprovedBuilders(message);

    ASSERT_EQ(response.builders.size(), 2u);
    EXPECT_EQ(response.builders[0], "0xbuilder1");
    EXPECT_EQ(response.builders[1], "0xbuilder2");
}

// --- Staking / delegation parsing ---

TEST(RestApiMessageParserInfoTest, ParseDelegations)
{
    std::string message = R"([
        {"validator": "0xvalidator1", "amount": "123.45", "lockedUntilTimestamp": 1690000000000}
    ])";

    RestApiMessageParser parser;
    auto response = parser.parseDelegations(message);

    ASSERT_EQ(response.delegations.size(), 1u);
    EXPECT_EQ(response.delegations[0].validator, "0xvalidator1");
    EXPECT_DOUBLE_EQ(response.delegations[0].amount, 123.45);
    EXPECT_EQ(response.delegations[0].lockedUntilTimestamp, 1690000000000ULL);
}

TEST(RestApiMessageParserInfoTest, ParseDelegatorSummary)
{
    std::string message = R"({
        "delegated": "10.5",
        "undelegated": "2.0",
        "totalPendingWithdrawal": "1.0",
        "nPendingWithdrawals": 2
    })";

    RestApiMessageParser parser;
    auto response = parser.parseDelegatorSummary(message);

    EXPECT_DOUBLE_EQ(response.delegated, 10.5);
    EXPECT_DOUBLE_EQ(response.undelegated, 2.0);
    EXPECT_DOUBLE_EQ(response.totalPendingWithdrawal, 1.0);
    EXPECT_EQ(response.nPendingWithdrawals, 2);
}

TEST(RestApiMessageParserInfoTest, ParseDelegatorHistory)
{
    std::string message = R"([
        {
            "time": 1690000000000,
            "hash": "0xhash1",
            "delta": {"delegate": {"validator": "0xvalidator1", "amount": "10.0", "isUndelegate": false}}
        },
        {
            "time": 1690000001000,
            "hash": "0xhash2",
            "delta": {"cDeposit": {"amount": "5.0"}}
        },
        {
            "time": 1690000002000,
            "hash": "0xhash3",
            "delta": {"withdrawal": {"amount": "3.0", "phase": "initiated"}}
        }
    ])";

    RestApiMessageParser parser;
    auto response = parser.parseDelegatorHistory(message);

    ASSERT_EQ(response.history.size(), 3u);

    EXPECT_EQ(response.history[0].hash, "0xhash1");
    EXPECT_EQ(response.history[0].delta.type, DelegatorHistoryDeltaType::Delegate);
    EXPECT_EQ(response.history[0].delta.validator, "0xvalidator1");
    EXPECT_DOUBLE_EQ(response.history[0].delta.amount, 10.0);
    EXPECT_FALSE(response.history[0].delta.isUndelegate);

    EXPECT_EQ(response.history[1].delta.type, DelegatorHistoryDeltaType::CDeposit);
    EXPECT_DOUBLE_EQ(response.history[1].delta.amount, 5.0);

    EXPECT_EQ(response.history[2].delta.type, DelegatorHistoryDeltaType::Withdrawal);
    EXPECT_DOUBLE_EQ(response.history[2].delta.amount, 3.0);
    EXPECT_EQ(response.history[2].delta.phase, WithdrawalPhase::Initiated);
}

TEST(RestApiMessageParserInfoTest, ParseDelegatorRewards)
{
    std::string message = R"([
        {"time": 1690000000000, "source": "delegation", "totalAmount": "1.23"},
        {"time": 1690000001000, "source": "commission", "totalAmount": "0.45"}
    ])";

    RestApiMessageParser parser;
    auto response = parser.parseDelegatorRewards(message);

    ASSERT_EQ(response.rewards.size(), 2u);
    EXPECT_EQ(response.rewards[0].source, DelegatorRewardSource::Delegation);
    EXPECT_DOUBLE_EQ(response.rewards[0].totalAmount, 1.23);
    EXPECT_EQ(response.rewards[1].source, DelegatorRewardSource::Commission);
    EXPECT_DOUBLE_EQ(response.rewards[1].totalAmount, 0.45);
}

// --- Borrow/lend ---

TEST(InfoRequestBuilderTest, BorrowLendUserState)
{
    auto body = InfoRequestBuilder::borrowLendUserState("0xabc");
    EXPECT_EQ(body["type"], "borrowLendUserState");
    EXPECT_EQ(body["user"], "0xabc");
}

TEST(InfoRequestBuilderTest, BorrowLendReserveState)
{
    auto body = InfoRequestBuilder::borrowLendReserveState(0);
    EXPECT_EQ(body["type"], "borrowLendReserveState");
    EXPECT_EQ(body["token"], 0);
}

TEST(InfoRequestBuilderTest, AllBorrowLendReserveStates)
{
    auto body = InfoRequestBuilder::allBorrowLendReserveStates();
    EXPECT_EQ(body["type"], "allBorrowLendReserveStates");
}

TEST(RestApiMessageParserInfoTest, ParseBorrowLendReserveState)
{
    std::string message = R"({
        "borrowYearlyRate": "0.05",
        "supplyYearlyRate": "0.0035124541",
        "balance": "4000064.8929032399",
        "utilization": "0.078054536",
        "oraclePx": "1.0",
        "ltv": "0.0",
        "totalSupplied": "4338721.1769786999",
        "totalBorrowed": "338656.86839526"
    })";

    RestApiMessageParser parser;
    auto response = parser.parseBorrowLendReserveState(message);

    EXPECT_DOUBLE_EQ(response.borrowYearlyRate, 0.05);
    EXPECT_DOUBLE_EQ(response.supplyYearlyRate, 0.0035124541);
    EXPECT_DOUBLE_EQ(response.balance, 4000064.8929032399);
    EXPECT_DOUBLE_EQ(response.utilization, 0.078054536);
    EXPECT_DOUBLE_EQ(response.oraclePx, 1.0);
    EXPECT_DOUBLE_EQ(response.ltv, 0.0);
    EXPECT_DOUBLE_EQ(response.totalSupplied, 4338721.1769786999);
    EXPECT_DOUBLE_EQ(response.totalBorrowed, 338656.86839526);
}

TEST(RestApiMessageParserInfoTest, ParseAllBorrowLendReserveStates)
{
    // Top level is an array of [tokenId, reserveStateObj] pairs, not a flat array of objects.
    std::string message = R"([
        [0, {"borrowYearlyRate": "0.05", "supplyYearlyRate": "0.0035124548", "balance": "4000064.8172154501", "utilization": "0.078054552", "oraclePx": "1.0", "ltv": "0.0", "totalSupplied": "4338721.1701327004", "totalBorrowed": "338656.93723639"}],
        [1, {"borrowYearlyRate": "0.05", "supplyYearlyRate": "0.0", "balance": "6945.8974", "utilization": "0.0", "oraclePx": "4.6252", "ltv": "0.5", "totalSupplied": "6927.0173", "totalBorrowed": "0.0"}]
    ])";

    RestApiMessageParser parser;
    auto response = parser.parseAllBorrowLendReserveStates(message);

    ASSERT_EQ(response.reserves.size(), 2u);
    EXPECT_EQ(response.reserves[0].token, 0);
    EXPECT_DOUBLE_EQ(response.reserves[0].state.totalSupplied, 4338721.1701327004);
    EXPECT_EQ(response.reserves[1].token, 1);
    EXPECT_DOUBLE_EQ(response.reserves[1].state.ltv, 0.5);
}

TEST(RestApiMessageParserInfoTest, ParseBorrowLendUserStateEmpty)
{
    std::string message = R"({"tokenToState":[],"health":"healthy","healthFactor":null})";

    RestApiMessageParser parser;
    auto response = parser.parseBorrowLendUserState(message);

    EXPECT_TRUE(response.tokenToState.empty());
    EXPECT_EQ(response.health, "healthy");
    EXPECT_FALSE(response.healthFactor.has_value());
}

TEST(RestApiMessageParserInfoTest, ParseBorrowLendUserStateWithPosition)
{
    // tokenToState is an array of [tokenId, {borrow, supply}] pairs, not a flat array of
    // {token, deposited, borrowed} objects.
    std::string message = R"({
        "tokenToState": [
            [0, {"borrow": {"basis": "0.0", "value": "0.0"}, "supply": {"basis": "2.0", "value": "1.99999999"}}]
        ],
        "health": "healthy",
        "healthFactor": null
    })";

    RestApiMessageParser parser;
    auto response = parser.parseBorrowLendUserState(message);

    ASSERT_EQ(response.tokenToState.size(), 1u);
    EXPECT_EQ(response.tokenToState[0].token, 0);
    EXPECT_DOUBLE_EQ(response.tokenToState[0].borrow.value, 0.0);
    EXPECT_DOUBLE_EQ(response.tokenToState[0].supply.basis, 2.0);
    EXPECT_DOUBLE_EQ(response.tokenToState[0].supply.value, 1.99999999);
    EXPECT_EQ(response.health, "healthy");
    EXPECT_FALSE(response.healthFactor.has_value());
}

TEST(RestApiMessageParserInfoTest, ParseBorrowLendUserStateHealthFactorPresent)
{
    // Synthetic - healthFactor is nullable but this covers the non-null case.
    std::string message = R"({"tokenToState":[],"health":"healthy","healthFactor":1.5})";

    RestApiMessageParser parser;
    auto response = parser.parseBorrowLendUserState(message);

    ASSERT_TRUE(response.healthFactor.has_value());
    EXPECT_DOUBLE_EQ(*response.healthFactor, 1.5);
}

// --- Settled outcome ---

TEST(InfoRequestBuilderTest, SettledOutcome)
{
    auto body = InfoRequestBuilder::settledOutcome(10234);
    EXPECT_EQ(body["type"], "settledOutcome");
    EXPECT_EQ(body["outcome"], 10234);
    EXPECT_FALSE(isAuthenticated(RestEndpointType::SettledOutcome));
}

TEST(RestApiMessageParserInfoTest, ParseSettledOutcomeNull)
{
    // Never-settled/unknown outcome ids resolve to a bare JSON null.
    std::string message = "null";

    RestApiMessageParser parser;
    auto response = parser.parseSettledOutcome(message);

    EXPECT_FALSE(response.isSettled);
}

TEST(RestApiMessageParserInfoTest, ParseSettledOutcomeNoQuestion)
{
    std::string message =
        R"({"spec":{"outcome":1000,"name":"Recurring","description":"class:priceBinary|underlying:HYPE|expiry:20260313-0730|targetPrice:37.87|period:15m","sideSpecs":[{"name":"Yes"},{"name":"No"}],"quoteToken":"USDH"},"settleFraction":"0.0","details":"price:37.87"})";

    RestApiMessageParser parser;
    auto response = parser.parseSettledOutcome(message);

    EXPECT_TRUE(response.isSettled);
    EXPECT_EQ(response.spec.outcome, 1000);
    EXPECT_EQ(response.spec.name, "Recurring");
    EXPECT_EQ(response.spec.description.underlying, "HYPE");
    EXPECT_DOUBLE_EQ(response.settleFraction, 0.0);
    EXPECT_EQ(response.details, "price:37.87");
    EXPECT_FALSE(response.question.has_value());
}

TEST(RestApiMessageParserInfoTest, ParseSettledOutcomeQuestionSettled)
{
    // question.question is keyed "settled" (not "active") once the linked question itself
    // has resolved.
    std::string message =
        R"({"spec":{"outcome":10100,"name":"Recurring Named Outcome","description":"index:2","sideSpecs":[{"name":"Yes"},{"name":"No"}],"quoteToken":"USDC"},"settleFraction":"0.0","details":"price:2144.1","question":{"question":{"settled":796},"name":"Recurring","description":"class:priceBucket|underlying:ETH|expiry:20260521-0845|priceThresholds:2105,2190.9|period:15m"}})";

    RestApiMessageParser parser;
    auto response = parser.parseSettledOutcome(message);

    ASSERT_TRUE(response.question.has_value());
    EXPECT_TRUE(response.question->isSettled);
    EXPECT_EQ(response.question->questionId, 796);
    EXPECT_EQ(response.question->name, "Recurring");
}

TEST(RestApiMessageParserInfoTest, ParseSettledOutcomeQuestionActive)
{
    // question.question is keyed "active" while the linked question (e.g. an ongoing
    // multi-outcome tournament winner) has not yet resolved, even though this particular
    // outcome (one eliminated contestant) has already settled to "No".
    std::string message =
        R"({"spec":{"outcome":10234,"name":"Australia","description":"This outcome resolves to Yes if Australia is officially declared the 2026 HYPURR World Cup champion.","sideSpecs":[{"name":"Yes"},{"name":"No"}],"quoteToken":"USDC"},"settleFraction":"0.0","details":"Australia was eliminated from the 2026 FIFA World Cup and can no longer win the tournament.","question":{"question":{"active":823},"name":"2026 World Cup champion","description":"metadata=category:sports|subCategory:football"}})";

    RestApiMessageParser parser;
    auto response = parser.parseSettledOutcome(message);

    ASSERT_TRUE(response.question.has_value());
    EXPECT_FALSE(response.question->isSettled);
    EXPECT_EQ(response.question->questionId, 823);
    EXPECT_EQ(response.question->name, "2026 World Cup champion");
}

// --- HIP-3 deployer (perp dex abstraction) ---

TEST(InfoRequestBuilderTest, PerpDexLimits)
{
    auto body = InfoRequestBuilder::perpDexLimits("hyna");
    EXPECT_EQ(body["type"], "perpDexLimits");
    EXPECT_EQ(body["dex"], "hyna");
}

TEST(InfoRequestBuilderTest, PerpDexStatus)
{
    auto body = InfoRequestBuilder::perpDexStatus("hyna");
    EXPECT_EQ(body["type"], "perpDexStatus");
    EXPECT_EQ(body["dex"], "hyna");
}

TEST(InfoRequestBuilderTest, PerpDeployAuctionStatus)
{
    // Unlike perpDexLimits/perpDexStatus, this endpoint takes no dex parameter - confirmed
    // against both the official TS SDK (@nktkas/hyperliquid) and real testnet responses,
    // which are identical whether or not a "dex" field is sent.
    auto body = InfoRequestBuilder::perpDeployAuctionStatus();
    EXPECT_EQ(body["type"], "perpDeployAuctionStatus");
    EXPECT_FALSE(body.contains("dex"));
}

TEST(RestApiMessageParserInfoTest, ParsePerpDexLimitsNull)
{
    // Real testnet response for the main dex (empty string) or an unknown dex name.
    std::string message = "null";

    RestApiMessageParser parser;
    auto response = parser.parsePerpDexLimits(message);

    EXPECT_FALSE(response.exists);
}

TEST(RestApiMessageParserInfoTest, ParsePerpDexLimits)
{
    // Real testnet response for a live HIP-3 dex ("hyna").
    std::string message =
        R"({"totalOiCap":"50000000.0","oiSzCapPerPerp":"10000000000.0","maxTransferNtl":"1000000000.0","coinToOiCap":[["hyna:BTC","100000.0"]]})";

    RestApiMessageParser parser;
    auto response = parser.parsePerpDexLimits(message);

    EXPECT_TRUE(response.exists);
    EXPECT_DOUBLE_EQ(response.totalOiCap, 50000000.0);
    EXPECT_DOUBLE_EQ(response.oiSzCapPerPerp, 10000000000.0);
    EXPECT_DOUBLE_EQ(response.maxTransferNtl, 1000000000.0);
    ASSERT_EQ(response.coinToOiCap.size(), 1u);
    EXPECT_EQ(response.coinToOiCap[0].coin, "hyna:BTC");
    EXPECT_DOUBLE_EQ(response.coinToOiCap[0].oiCap, 100000.0);
}

TEST(RestApiMessageParserInfoTest, ParsePerpDexStatusNull)
{
    // Real testnet response for an unknown dex name.
    std::string message = "null";

    RestApiMessageParser parser;
    auto response = parser.parsePerpDexStatus(message);

    EXPECT_FALSE(response.exists);
}

TEST(RestApiMessageParserInfoTest, ParsePerpDexStatus)
{
    // Real testnet response for a live HIP-3 dex ("hyna").
    std::string message = R"({"totalNetDeposit":"2518.912535"})";

    RestApiMessageParser parser;
    auto response = parser.parsePerpDexStatus(message);

    EXPECT_TRUE(response.exists);
    EXPECT_DOUBLE_EQ(response.totalNetDeposit, 2518.912535);
}

TEST(RestApiMessageParserInfoTest, ParsePerpDeployAuctionStatusEndGasNull)
{
    // Real testnet response - the auction is still in progress, so endGas is null.
    std::string message = R"({"startTimeSeconds":1788613200,"durationSeconds":111600,"startGas":"500.0","currentGas":"500.0","endGas":null})";

    RestApiMessageParser parser;
    auto response = parser.parsePerpDeployAuctionStatus(message);

    EXPECT_EQ(response.startTimeSeconds, 1788613200ULL);
    EXPECT_EQ(response.durationSeconds, 111600ULL);
    EXPECT_DOUBLE_EQ(response.startGas, 500.0);
    ASSERT_TRUE(response.currentGas.has_value());
    EXPECT_DOUBLE_EQ(*response.currentGas, 500.0);
    EXPECT_FALSE(response.endGas.has_value());
}

TEST(RestApiMessageParserInfoTest, ParsePerpDeployAuctionStatusEndGasPresent)
{
    // Synthetic - covers the completed-auction case where endGas is a non-null string.
    std::string message = R"({"startTimeSeconds":1690000000,"durationSeconds":3600,"startGas":"1000000.0","currentGas":null,"endGas":"100000.0"})";

    RestApiMessageParser parser;
    auto response = parser.parsePerpDeployAuctionStatus(message);

    EXPECT_FALSE(response.currentGas.has_value());
    ASSERT_TRUE(response.endGas.has_value());
    EXPECT_DOUBLE_EQ(*response.endGas, 100000.0);
}
