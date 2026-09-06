#include <gtest/gtest.h>

#include "messages/InfoRequestBuilder.h"
#include "hyperliquid/rest/RestApiMessageParser.h"

#include <nlohmann/json.hpp>

using namespace hyperliquid;

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
