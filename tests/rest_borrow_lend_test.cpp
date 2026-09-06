#include <gtest/gtest.h>

#include "messages/ExchangeRequestBuilder.h"
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

// --- borrowLend exchange action (write side) ---
//
// Field shapes below (action = {type, operation, token, amount}, plain L1-action signing with no
// embedded nonce/hyperliquidChain fields) are verified against the official TS SDK
// (@nktkas/hyperliquid), not a live capture - this action isn't in the public gitbook docs.

TEST(BorrowLendBuilder, SupplyWithAmountIncludesAllFieldsInOrder)
{
    ExchangeRequestBuilder builder;

    BorrowLendRequest request;
    request.operation = BorrowLendOperation::Supply;
    request.token = 0;
    request.amount = 1.5;

    auto body = builder.borrowLend(request);

    ASSERT_TRUE(body.contains("action"));
    const auto& action = body["action"];
    EXPECT_EQ(action["type"], "borrowLend");
    EXPECT_EQ(action["operation"], "supply");
    EXPECT_EQ(action["token"].get<uint32_t>(), 0u);
    EXPECT_EQ(action["amount"], "1.5");

    auto keys = std::vector<std::string>();
    for (auto it = action.begin(); it != action.end(); ++it) keys.push_back(it.key());
    EXPECT_EQ(keys, (std::vector<std::string>{"type", "operation", "token", "amount"}));
}

TEST(BorrowLendBuilder, WithdrawWithNulloptAmountSendsExplicitNull)
{
    ExchangeRequestBuilder builder;

    BorrowLendRequest request;
    request.operation = BorrowLendOperation::Withdraw;
    request.token = 3;
    request.amount = std::nullopt;

    auto body = builder.borrowLend(request);

    const auto& action = body["action"];
    EXPECT_EQ(action["operation"], "withdraw");
    EXPECT_EQ(action["token"].get<uint32_t>(), 3u);
    ASSERT_TRUE(action.contains("amount"));
    EXPECT_TRUE(action["amount"].is_null());
}

TEST(BorrowLendBuilder, RepaySetsOperation)
{
    ExchangeRequestBuilder builder;

    BorrowLendRequest request;
    request.operation = BorrowLendOperation::Repay;
    request.token = 1;
    request.amount = 10.0;

    auto body = builder.borrowLend(request);

    const auto& action = body["action"];
    EXPECT_EQ(action["operation"], "repay");
    EXPECT_EQ(action["amount"], "10");
}

TEST(BorrowLendBuilder, BorrowSetsOperation)
{
    ExchangeRequestBuilder builder;

    BorrowLendRequest request;
    request.operation = BorrowLendOperation::Borrow;
    request.token = 2;
    request.amount = 25.25;

    auto body = builder.borrowLend(request);

    const auto& action = body["action"];
    EXPECT_EQ(action["operation"], "borrow");
    EXPECT_EQ(action["amount"], "25.25");
}

TEST(BorrowLendResponseParsing, SuccessResponse)
{
    // Real testnet response: confirmed live via a supply-then-withdraw pair of $1 USDC (token 0),
    // both of which returned this exact shape (see examples/rest_borrow_lend_action.cpp).
    static const std::string kOk = R"({
        "status": "ok",
        "response": {
            "type": "default"
        }
    })";

    RestApiMessageParser parser;
    auto resp = parser.parseSimpleResponse(kOk);

    EXPECT_EQ(resp.status, "ok");
    EXPECT_FALSE(resp.error.has_value());
}

TEST(BorrowLendResponseParsing, ErrorResponse)
{
    // Synthetic but plausible - shape (status/response error string) verified against other
    // rejected L1 actions in this codebase (e.g. hip3LiquidatorTransfer's insufficient-funds
    // response); not a live capture for borrowLend specifically.
    static const std::string kErr = R"({
        "status": "err",
        "response": "Insufficient balance to supply."
    })";

    RestApiMessageParser parser;
    auto resp = parser.parseSimpleResponse(kErr);

    EXPECT_EQ(resp.status, "err");
    ASSERT_TRUE(resp.error.has_value());
    EXPECT_EQ(*resp.error, "Insufficient balance to supply.");
}
