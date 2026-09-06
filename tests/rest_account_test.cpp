#include <gtest/gtest.h>

#include "messages/InfoRequestBuilder.h"
#include "hyperliquid/rest/RestApiMessageParser.h"

#include <nlohmann/json.hpp>

using namespace hyperliquid;

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
