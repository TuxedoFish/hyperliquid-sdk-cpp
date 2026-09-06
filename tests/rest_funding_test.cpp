#include <gtest/gtest.h>

#include "messages/InfoRequestBuilder.h"
#include "hyperliquid/rest/RestApiMessageParser.h"

#include <nlohmann/json.hpp>

using namespace hyperliquid;

TEST(InfoRequestBuilderTest, UserFundingMinimal)
{
    auto body = InfoRequestBuilder::userFunding("0xabc", 1681222254710ULL);
    EXPECT_EQ(body["type"], "userFunding");
    EXPECT_EQ(body["user"], "0xabc");
    EXPECT_EQ(body["startTime"], 1681222254710ULL);
    EXPECT_FALSE(body.contains("endTime"));
}

TEST(InfoRequestBuilderTest, UserFundingWithEndTime)
{
    auto body = InfoRequestBuilder::userFunding("0xabc", 1681222254710ULL, 1681223254710ULL);
    EXPECT_EQ(body["type"], "userFunding");
    EXPECT_EQ(body["user"], "0xabc");
    EXPECT_EQ(body["startTime"], 1681222254710ULL);
    EXPECT_EQ(body["endTime"], 1681223254710ULL);
}

TEST(InfoRequestBuilderTest, UserNonFundingLedgerUpdatesMinimal)
{
    auto body = InfoRequestBuilder::userNonFundingLedgerUpdates("0xabc", 1681222254710ULL);
    EXPECT_EQ(body["type"], "userNonFundingLedgerUpdates");
    EXPECT_EQ(body["user"], "0xabc");
    EXPECT_EQ(body["startTime"], 1681222254710ULL);
    EXPECT_FALSE(body.contains("endTime"));
}

TEST(InfoRequestBuilderTest, UserNonFundingLedgerUpdatesWithEndTime)
{
    auto body = InfoRequestBuilder::userNonFundingLedgerUpdates("0xabc", 1681222254710ULL, 1681223254710ULL);
    EXPECT_EQ(body["type"], "userNonFundingLedgerUpdates");
    EXPECT_EQ(body["endTime"], 1681223254710ULL);
}

TEST(InfoRequestBuilderTest, FundingHistoryMinimal)
{
    auto body = InfoRequestBuilder::fundingHistory("ETH", 1681222254710ULL);
    EXPECT_EQ(body["type"], "fundingHistory");
    EXPECT_EQ(body["coin"], "ETH");
    EXPECT_EQ(body["startTime"], 1681222254710ULL);
    EXPECT_FALSE(body.contains("endTime"));
}

TEST(InfoRequestBuilderTest, FundingHistoryWithEndTime)
{
    auto body = InfoRequestBuilder::fundingHistory("ETH", 1681222254710ULL, 1681223254710ULL);
    EXPECT_EQ(body["type"], "fundingHistory");
    EXPECT_EQ(body["endTime"], 1681223254710ULL);
}

TEST(RestApiMessageParserInfoTest, ParseUserFunding)
{
    std::string message = R"([
        {
            "delta": {"coin": "ETH", "fundingRate": "0.0000417", "szi": "49.1477", "type": "funding", "usdc": "-3.625312", "nSamples": null},
            "hash": "0xa166e3fa63c25663024b03f2e0da011a00307e4017465df020210d3d432e7cb8",
            "time": 1681222254710
        }
    ])";

    RestApiMessageParser parser;
    auto response = parser.parseUserFunding(message);

    ASSERT_EQ(response.updates.size(), 1u);
    const auto& entry = response.updates[0];
    EXPECT_EQ(entry.time, 1681222254710ULL);
    EXPECT_EQ(entry.hash, "0xa166e3fa63c25663024b03f2e0da011a00307e4017465df020210d3d432e7cb8");
    EXPECT_EQ(entry.delta.type, LedgerUpdateDeltaType::Funding);
    ASSERT_TRUE(entry.delta.coin.has_value());
    EXPECT_EQ(*entry.delta.coin, "ETH");
    ASSERT_TRUE(entry.delta.fundingRate.has_value());
    EXPECT_DOUBLE_EQ(*entry.delta.fundingRate, 0.0000417);
    ASSERT_TRUE(entry.delta.szi.has_value());
    EXPECT_DOUBLE_EQ(*entry.delta.szi, 49.1477);
    ASSERT_TRUE(entry.delta.usdc.has_value());
    EXPECT_DOUBLE_EQ(*entry.delta.usdc, -3.625312);
    EXPECT_FALSE(entry.delta.nSamples.has_value());
}

TEST(RestApiMessageParserInfoTest, ParseUserFundingWithNSamples)
{
    std::string message = R"([
        {
            "delta": {"coin": "BTC", "fundingRate": "0.0001", "szi": "1.5", "type": "funding", "usdc": "0.5", "nSamples": 3},
            "hash": "0xhash1",
            "time": 1681222254711
        }
    ])";

    RestApiMessageParser parser;
    auto response = parser.parseUserFunding(message);

    ASSERT_EQ(response.updates.size(), 1u);
    ASSERT_TRUE(response.updates[0].delta.nSamples.has_value());
    EXPECT_DOUBLE_EQ(*response.updates[0].delta.nSamples, 3.0);
}

TEST(RestApiMessageParserInfoTest, ParseUserNonFundingLedgerUpdatesDeposit)
{
    std::string message = R"([
        {
            "delta": {"type": "deposit", "usdc": "100.0"},
            "hash": "0xdeposit1",
            "time": 1681222254710
        }
    ])";

    RestApiMessageParser parser;
    auto response = parser.parseUserNonFundingLedgerUpdates(message);

    ASSERT_EQ(response.updates.size(), 1u);
    const auto& delta = response.updates[0].delta;
    EXPECT_EQ(delta.type, LedgerUpdateDeltaType::Deposit);
    ASSERT_TRUE(delta.usdc.has_value());
    EXPECT_DOUBLE_EQ(*delta.usdc, 100.0);
}

TEST(RestApiMessageParserInfoTest, ParseUserNonFundingLedgerUpdatesWithdraw)
{
    std::string message = R"([
        {
            "delta": {"type": "withdraw", "usdc": "-50.0", "nonce": 12345, "fee": "1.0"},
            "hash": "0xwithdraw1",
            "time": 1681222254720
        }
    ])";

    RestApiMessageParser parser;
    auto response = parser.parseUserNonFundingLedgerUpdates(message);

    ASSERT_EQ(response.updates.size(), 1u);
    const auto& delta = response.updates[0].delta;
    EXPECT_EQ(delta.type, LedgerUpdateDeltaType::Withdraw);
    ASSERT_TRUE(delta.usdc.has_value());
    EXPECT_DOUBLE_EQ(*delta.usdc, -50.0);
    ASSERT_TRUE(delta.nonce.has_value());
    EXPECT_EQ(*delta.nonce, 12345ULL);
    ASSERT_TRUE(delta.fee.has_value());
    EXPECT_DOUBLE_EQ(*delta.fee, 1.0);
}

TEST(RestApiMessageParserInfoTest, ParseUserNonFundingLedgerUpdatesAccountClassTransfer)
{
    std::string message = R"([
        {
            "delta": {"type": "accountClassTransfer", "usdc": "25.0", "toPerp": true},
            "hash": "0xtransfer1",
            "time": 1681222254730
        }
    ])";

    RestApiMessageParser parser;
    auto response = parser.parseUserNonFundingLedgerUpdates(message);

    ASSERT_EQ(response.updates.size(), 1u);
    const auto& delta = response.updates[0].delta;
    EXPECT_EQ(delta.type, LedgerUpdateDeltaType::AccountClassTransfer);
    ASSERT_TRUE(delta.usdc.has_value());
    EXPECT_DOUBLE_EQ(*delta.usdc, 25.0);
    ASSERT_TRUE(delta.toPerp.has_value());
    EXPECT_TRUE(*delta.toPerp);
}

TEST(RestApiMessageParserInfoTest, ParseUserNonFundingLedgerUpdatesInternalTransfer)
{
    std::string message = R"([
        {
            "delta": {"type": "internalTransfer", "usdc": "10.0", "user": "0xsender", "destination": "0xreceiver", "fee": "0.1"},
            "hash": "0xinternal1",
            "time": 1681222254740
        }
    ])";

    RestApiMessageParser parser;
    auto response = parser.parseUserNonFundingLedgerUpdates(message);

    ASSERT_EQ(response.updates.size(), 1u);
    const auto& delta = response.updates[0].delta;
    EXPECT_EQ(delta.type, LedgerUpdateDeltaType::InternalTransfer);
    ASSERT_TRUE(delta.user.has_value());
    EXPECT_EQ(*delta.user, "0xsender");
    ASSERT_TRUE(delta.destination.has_value());
    EXPECT_EQ(*delta.destination, "0xreceiver");
    ASSERT_TRUE(delta.fee.has_value());
    EXPECT_DOUBLE_EQ(*delta.fee, 0.1);
}

TEST(RestApiMessageParserInfoTest, ParseUserNonFundingLedgerUpdatesSubAccountTransfer)
{
    std::string message = R"([
        {
            "delta": {"type": "subAccountTransfer", "usdc": "5.0", "user": "0xmaster", "destination": "0xsub"},
            "hash": "0xsub1",
            "time": 1681222254750
        }
    ])";

    RestApiMessageParser parser;
    auto response = parser.parseUserNonFundingLedgerUpdates(message);

    ASSERT_EQ(response.updates.size(), 1u);
    const auto& delta = response.updates[0].delta;
    EXPECT_EQ(delta.type, LedgerUpdateDeltaType::SubAccountTransfer);
    ASSERT_TRUE(delta.user.has_value());
    EXPECT_EQ(*delta.user, "0xmaster");
    ASSERT_TRUE(delta.destination.has_value());
    EXPECT_EQ(*delta.destination, "0xsub");
}

TEST(RestApiMessageParserInfoTest, ParseUserNonFundingLedgerUpdatesSpotTransfer)
{
    std::string message = R"([
        {
            "delta": {
                "type": "spotTransfer",
                "token": "PURR",
                "amount": "12.5",
                "usdcValue": "3.75",
                "user": "0xsender",
                "destination": "0xreceiver",
                "fee": "0.01",
                "nativeTokenFee": "0.0"
            },
            "hash": "0xspot1",
            "time": 1681222254760
        }
    ])";

    RestApiMessageParser parser;
    auto response = parser.parseUserNonFundingLedgerUpdates(message);

    ASSERT_EQ(response.updates.size(), 1u);
    const auto& delta = response.updates[0].delta;
    EXPECT_EQ(delta.type, LedgerUpdateDeltaType::SpotTransfer);
    ASSERT_TRUE(delta.token.has_value());
    EXPECT_EQ(*delta.token, "PURR");
    ASSERT_TRUE(delta.amount.has_value());
    EXPECT_DOUBLE_EQ(*delta.amount, 12.5);
    ASSERT_TRUE(delta.usdcValue.has_value());
    EXPECT_DOUBLE_EQ(*delta.usdcValue, 3.75);
}

TEST(RestApiMessageParserInfoTest, ParseUserNonFundingLedgerUpdatesUnknownFallback)
{
    std::string message = R"([
        {
            "delta": {"type": "vaultDeposit", "vault": "0xvault1", "usdc": "42.0"},
            "hash": "0xvault1hash",
            "time": 1681222254770
        }
    ])";

    RestApiMessageParser parser;
    auto response = parser.parseUserNonFundingLedgerUpdates(message);

    ASSERT_EQ(response.updates.size(), 1u);
    const auto& delta = response.updates[0].delta;
    EXPECT_EQ(delta.type, LedgerUpdateDeltaType::Unknown);
    ASSERT_TRUE(delta.rawJson.has_value());
    EXPECT_NE(delta.rawJson->find("vaultDeposit"), std::string::npos);
}

TEST(RestApiMessageParserInfoTest, ParseFundingHistory)
{
    std::string message = R"([
        {"coin": "ETH", "fundingRate": "-0.00022196", "premium": "-0.00052196", "time": 1683849600076}
    ])";

    RestApiMessageParser parser;
    auto response = parser.parseFundingHistory(message);

    ASSERT_EQ(response.history.size(), 1u);
    EXPECT_EQ(response.history[0].coin, "ETH");
    EXPECT_DOUBLE_EQ(response.history[0].fundingRate, -0.00022196);
    EXPECT_DOUBLE_EQ(response.history[0].premium, -0.00052196);
    EXPECT_EQ(response.history[0].time, 1683849600076ULL);
}
