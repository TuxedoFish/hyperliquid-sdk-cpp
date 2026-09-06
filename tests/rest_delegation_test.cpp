#include <gtest/gtest.h>

#include "messages/InfoRequestBuilder.h"
#include "hyperliquid/rest/RestApiMessageParser.h"

#include <nlohmann/json.hpp>

using namespace hyperliquid;

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
