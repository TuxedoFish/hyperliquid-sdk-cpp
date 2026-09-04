#include <gtest/gtest.h>

#include "hyperliquid/rest/RestApiMessageParser.h"

using namespace hyperliquid;

TEST(RestResponseParsing, PlaceOrderResting)
{
    static const std::string kResting = R"({
        "status": "ok",
        "response": {
            "type": "order",
            "data": {
                "statuses": [
                    {"resting": {"oid": 77738308}}
                ]
            }
        }
    })";

    RestApiMessageParser parser;
    auto resp = parser.parsePlaceOrder(kResting);

    EXPECT_EQ(resp.status, "ok");
    EXPECT_EQ(resp.type, "order");
    ASSERT_EQ(resp.statuses.size(), 1u);

    const auto& status = resp.statuses[0];
    ASSERT_TRUE(status.resting.has_value());
    EXPECT_FALSE(status.filled.has_value());
    EXPECT_FALSE(status.error.has_value());
    EXPECT_EQ(status.resting->oid, 77738308u);
    EXPECT_FALSE(status.resting->cloid.has_value());
}

TEST(RestResponseParsing, PlaceOrderRestingWithCloid)
{
    static const std::string kResting = R"({
        "status": "ok",
        "response": {
            "type": "order",
            "data": {
                "statuses": [
                    {"resting": {"oid": 5, "cloid": "0xdeadbeef"}}
                ]
            }
        }
    })";

    RestApiMessageParser parser;
    auto resp = parser.parsePlaceOrder(kResting);

    ASSERT_EQ(resp.statuses.size(), 1u);
    ASSERT_TRUE(resp.statuses[0].resting.has_value());
    EXPECT_EQ(resp.statuses[0].resting->oid, 5u);
    ASSERT_TRUE(resp.statuses[0].resting->cloid.has_value());
    EXPECT_EQ(*resp.statuses[0].resting->cloid, "0xdeadbeef");
}

TEST(RestResponseParsing, PlaceOrderFilled)
{
    static const std::string kFilled = R"({
        "status": "ok",
        "response": {
            "type": "order",
            "data": {
                "statuses": [
                    {"filled": {"totalSz": "0.02", "avgPx": "1891.4", "oid": 77747314}}
                ]
            }
        }
    })";

    RestApiMessageParser parser;
    auto resp = parser.parsePlaceOrder(kFilled);

    ASSERT_EQ(resp.statuses.size(), 1u);
    const auto& status = resp.statuses[0];
    EXPECT_FALSE(status.resting.has_value());
    ASSERT_TRUE(status.filled.has_value());
    EXPECT_EQ(status.filled->totalSz, "0.02");
    EXPECT_EQ(status.filled->avgPx, "1891.4");
    EXPECT_EQ(status.filled->oid, 77747314u);
}

TEST(RestResponseParsing, PlaceOrderPerStatusError)
{
    static const std::string kError = R"({
        "status": "ok",
        "response": {
            "type": "order",
            "data": {
                "statuses": [
                    {"error": "Insufficient margin to place order."}
                ]
            }
        }
    })";

    RestApiMessageParser parser;
    auto resp = parser.parsePlaceOrder(kError);

    ASSERT_EQ(resp.statuses.size(), 1u);
    const auto& status = resp.statuses[0];
    EXPECT_FALSE(status.resting.has_value());
    EXPECT_FALSE(status.filled.has_value());
    ASSERT_TRUE(status.error.has_value());
    EXPECT_EQ(*status.error, "Insufficient margin to place order.");
}

TEST(RestResponseParsing, PlaceOrderTopLevelError)
{
    static const std::string kErr = R"({
        "status": "err",
        "response": "Invalid signature"
    })";

    RestApiMessageParser parser;
    auto resp = parser.parsePlaceOrder(kErr);

    EXPECT_EQ(resp.status, "err");
    EXPECT_TRUE(resp.type.empty());
    EXPECT_TRUE(resp.statuses.empty());
}

TEST(RestResponseParsing, CancelOrderSuccess)
{
    static const std::string kSuccess = R"({
        "status": "ok",
        "response": {
            "type": "cancel",
            "data": {
                "statuses": ["success"]
            }
        }
    })";

    RestApiMessageParser parser;
    auto resp = parser.parseCancelOrder(kSuccess);

    EXPECT_EQ(resp.status, "ok");
    EXPECT_EQ(resp.type, "cancel");
    ASSERT_EQ(resp.statuses.size(), 1u);
    ASSERT_TRUE(resp.statuses[0].success.has_value());
    EXPECT_EQ(*resp.statuses[0].success, "success");
    EXPECT_FALSE(resp.statuses[0].error.has_value());
}

TEST(RestResponseParsing, CancelOrderError)
{
    static const std::string kError = R"({
        "status": "ok",
        "response": {
            "type": "cancel",
            "data": {
                "statuses": [
                    {"error": "Order was never placed, already canceled, or filled."}
                ]
            }
        }
    })";

    RestApiMessageParser parser;
    auto resp = parser.parseCancelOrder(kError);

    ASSERT_EQ(resp.statuses.size(), 1u);
    EXPECT_FALSE(resp.statuses[0].success.has_value());
    ASSERT_TRUE(resp.statuses[0].error.has_value());
    EXPECT_EQ(*resp.statuses[0].error, "Order was never placed, already canceled, or filled.");
}

TEST(RestResponseParsing, ModifyOrderSuccess)
{
    static const std::string kModify = R"({
        "status": "ok",
        "response": {
            "type": "default",
            "data": {}
        }
    })";

    RestApiMessageParser parser;
    auto resp = parser.parseModifyOrder(kModify);

    EXPECT_EQ(resp.status, "ok");
    EXPECT_EQ(resp.type, "default");
    EXPECT_TRUE(resp.statuses.empty());
}

TEST(RestResponseParsing, SimpleResponseOk)
{
    static const std::string kOk = R"({
        "status": "ok",
        "response": {
            "type": "default"
        }
    })";

    RestApiMessageParser parser;
    auto resp = parser.parseSimpleResponse(kOk);

    EXPECT_EQ(resp.status, "ok");
    EXPECT_EQ(resp.type, "default");
    EXPECT_FALSE(resp.error.has_value());
}

TEST(RestResponseParsing, SimpleResponseError)
{
    static const std::string kErr = R"({
        "status": "err",
        "response": "Invalid leverage value"
    })";

    RestApiMessageParser parser;
    auto resp = parser.parseSimpleResponse(kErr);

    EXPECT_EQ(resp.status, "err");
    EXPECT_TRUE(resp.type.empty());
    ASSERT_TRUE(resp.error.has_value());
    EXPECT_EQ(*resp.error, "Invalid leverage value");
}

TEST(RestResponseParsing, MetaResponse)
{
    static const std::string kMeta = R"({
        "universe": [
            {"name": "BTC", "szDecimals": 5, "maxLeverage": 50},
            {"name": "ETH", "szDecimals": 4, "maxLeverage": 50}
        ]
    })";

    RestApiMessageParser parser;
    auto resp = parser.parseMeta(kMeta);

    ASSERT_EQ(resp.universe.size(), 2u);
    EXPECT_EQ(resp.universe[0].name, "BTC");
    EXPECT_EQ(resp.universe[0].szDecimals, 5);
    EXPECT_EQ(resp.universe[0].maxLeverage, 50);
    EXPECT_EQ(resp.universe[1].name, "ETH");
}
