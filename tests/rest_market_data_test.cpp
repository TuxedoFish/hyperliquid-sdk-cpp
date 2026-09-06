#include <gtest/gtest.h>

#include "messages/InfoRequestBuilder.h"
#include "hyperliquid/rest/RestApiMessageParser.h"

#include <nlohmann/json.hpp>

using namespace hyperliquid;

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
