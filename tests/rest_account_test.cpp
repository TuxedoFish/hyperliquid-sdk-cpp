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

TEST(InfoRequestBuilderTest, UserTwapSliceFillsByTime)
{
    auto body = InfoRequestBuilder::userTwapSliceFillsByTime("0xabc", 1681222254710ULL, 1681223254710ULL, true);
    EXPECT_EQ(body["type"], "userTwapSliceFillsByTime");
    EXPECT_EQ(body["user"], "0xabc");
    EXPECT_EQ(body["startTime"], 1681222254710ULL);
    EXPECT_EQ(body["endTime"], 1681223254710ULL);
    EXPECT_EQ(body["aggregateByTime"], true);
}

TEST(InfoRequestBuilderTest, UserTwapSliceFillsByTimeStartOnly)
{
    auto body = InfoRequestBuilder::userTwapSliceFillsByTime("0xabc", 1681222254710ULL);
    EXPECT_EQ(body["startTime"], 1681222254710ULL);
    EXPECT_FALSE(body.contains("endTime"));
    EXPECT_FALSE(body.contains("aggregateByTime"));
}

TEST(InfoRequestBuilderTest, TwapHistory)
{
    auto body = InfoRequestBuilder::twapHistory("0xabc");
    EXPECT_EQ(body["type"], "twapHistory");
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

// userTwapSliceFillsByTime shares the exact array-of-{fill,twapId} schema as
// userTwapSliceFills (confirmed live against mainnet with a real TWAP trader address),
// just filtered by time range.
TEST(RestApiMessageParserInfoTest, ParseUserTwapSliceFillsByTime)
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
    auto response = parser.parseUserTwapSliceFillsByTime(message);

    ASSERT_EQ(response.fills.size(), 1u);
    EXPECT_EQ(response.fills[0].fill.coin, "ETH");
    EXPECT_EQ(response.fills[0].twapId, 42);
}

// Response shape confirmed live against mainnet (POST /info {"type":"twapHistory","user":..}) -
// a flat array of {time, state, status, twapId}, field order exactly as below. "state" is the
// same shape as the twapStates/userTwapHistory websocket channels' TwapState, plus "trigger"/
// "stopPx" fields (trigger-based TWAP orders) not yet modeled here - safe to leave unread since
// they're the last fields in the object. "status" is either {"status": "..."} or, for the "error"
// status, {"status": "error", "description": "..."}. All five status strings below
// (activated/terminated/waitingForTrigger/stopped/finished) plus "error" were observed live.
TEST(RestApiMessageParserInfoTest, ParseTwapHistory)
{
    std::string message = R"([
        {
            "time": 1788773946,
            "state": {
                "coin": "HYPE", "user": "0x6973a383b202b4349d256bbf8b0187dc7b2ed6bb", "side": "B",
                "sz": "11.45", "executedSz": "0.0", "executedNtl": "0.0", "minutes": 10080,
                "reduceOnly": false, "randomize": false, "timestamp": 1788773946267,
                "trigger": null, "stopPx": null
            },
            "status": {"status": "activated"},
            "twapId": 2193307
        },
        {
            "time": 1788843382,
            "state": {
                "coin": "HYPE", "user": "0x9b3cafa1209ac61f02d7bc3b219697fb171c9c91", "side": "B",
                "sz": "10.0", "executedSz": "0.0", "executedNtl": "0.0", "minutes": 4350,
                "reduceOnly": false, "randomize": false, "timestamp": 1788830253130,
                "trigger": {"px": "80.0", "above": false}, "stopPx": "85.0"
            },
            "status": {"status": "waitingForTrigger"},
            "twapId": 2195243
        },
        {
            "time": 1787497839,
            "state": {
                "coin": "HYPE", "user": "0x13c50dcdee4bbcba71baf578b345cdd35c7928be", "side": "A",
                "sz": "1535655.0", "executedSz": "0.0", "executedNtl": "0.0", "minutes": 60,
                "reduceOnly": false, "randomize": false, "timestamp": 1787497839000,
                "trigger": null, "stopPx": null
            },
            "status": {"status": "error", "description": "Insufficient spot balance"},
            "twapId": 1535655
        }
    ])";

    RestApiMessageParser parser;
    auto response = parser.parseTwapHistory(message);

    ASSERT_EQ(response.history.size(), 3u);

    EXPECT_EQ(response.history[0].time, 1788773946ULL);
    EXPECT_EQ(response.history[0].twapId, 2193307ULL);
    EXPECT_EQ(response.history[0].status, TwapHistoryStatus::Activated);
    EXPECT_EQ(response.history[0].state.coin, "HYPE");
    EXPECT_DOUBLE_EQ(response.history[0].state.sz, 11.45);
    EXPECT_EQ(response.history[0].state.side, 'B');

    EXPECT_EQ(response.history[1].status, TwapHistoryStatus::WaitingForTrigger);
    EXPECT_EQ(response.history[1].twapId, 2195243ULL);

    EXPECT_EQ(response.history[2].status, TwapHistoryStatus::Error);
    EXPECT_EQ(response.history[2].description, "Insufficient spot balance");
    EXPECT_EQ(response.history[2].twapId, 1535655ULL);
}

// All six statuses observed live against mainnet twapHistory responses across several real
// TWAP trader addresses (see PR description for the accounts/timestamps queried).
TEST(TwapHistoryStatusTest, AllObservedLiveValuesRoundTrip)
{
    EXPECT_EQ(stringToTwapHistoryStatus("activated"), TwapHistoryStatus::Activated);
    EXPECT_EQ(stringToTwapHistoryStatus("terminated"), TwapHistoryStatus::Terminated);
    EXPECT_EQ(stringToTwapHistoryStatus("waitingForTrigger"), TwapHistoryStatus::WaitingForTrigger);
    EXPECT_EQ(stringToTwapHistoryStatus("stopped"), TwapHistoryStatus::Stopped);
    EXPECT_EQ(stringToTwapHistoryStatus("finished"), TwapHistoryStatus::Finished);
    EXPECT_EQ(stringToTwapHistoryStatus("error"), TwapHistoryStatus::Error);
    EXPECT_EQ(stringToTwapHistoryStatus("somethingNew"), TwapHistoryStatus::Unknown);
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
