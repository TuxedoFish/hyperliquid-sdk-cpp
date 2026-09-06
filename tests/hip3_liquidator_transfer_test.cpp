#include <gtest/gtest.h>

#include "messages/ExchangeRequestBuilder.h"
#include "hyperliquid/rest/RestApiMessageParser.h"

using namespace hyperliquid;

// Field shapes below (action = {type, dex, ntl, isDeposit}, plain L1-action signing with no
// embedded nonce/hyperliquidChain fields) are verified against the official TS SDK
// (@nktkas/hyperliquid, src/api/exchange/_methods/hip3LiquidatorTransfer.ts), not a live capture.

TEST(Hip3LiquidatorTransferBuilder, DepositIncludesAllFieldsInOrder)
{
    ExchangeRequestBuilder builder;

    Hip3LiquidatorTransferRequest request;
    request.dex = "test";
    request.ntl = 1'000'000'000ULL;
    request.isDeposit = true;

    auto body = builder.hip3LiquidatorTransfer(request);

    ASSERT_TRUE(body.contains("action"));
    const auto& action = body["action"];
    EXPECT_EQ(action["type"], "hip3LiquidatorTransfer");
    EXPECT_EQ(action["dex"], "test");
    EXPECT_EQ(action["ntl"].get<uint64_t>(), 1'000'000'000ULL);
    EXPECT_EQ(action["isDeposit"].get<bool>(), true);

    auto keys = std::vector<std::string>();
    for (auto it = action.begin(); it != action.end(); ++it) keys.push_back(it.key());
    EXPECT_EQ(keys, (std::vector<std::string>{"type", "dex", "ntl", "isDeposit"}));
}

TEST(Hip3LiquidatorTransferBuilder, WithdrawalSetsIsDepositFalse)
{
    ExchangeRequestBuilder builder;

    Hip3LiquidatorTransferRequest request;
    request.dex = "test";
    request.ntl = 1'000'000'000ULL;
    request.isDeposit = false;

    auto body = builder.hip3LiquidatorTransfer(request);

    const auto& action = body["action"];
    EXPECT_EQ(action["isDeposit"].get<bool>(), false);
}

TEST(Hip3LiquidatorTransferResponseParsing, SuccessResponse)
{
    // Synthetic - shape verified against the official TS SDK. Depositing against this wallet on
    // testnet reliably hits ErrorResponse's real rejection below instead (see
    // examples/rest_hip3_liquidator_transfer.cpp), so the success shape isn't live-captured.
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

TEST(Hip3LiquidatorTransferResponseParsing, ErrorResponse)
{
    // Real testnet response: depositing 1000 quote tokens against the "test" HIP-3 dex from a
    // wallet with insufficient perp balance (see examples/rest_hip3_liquidator_transfer.cpp).
    static const std::string kErr = R"({
        "status": "err",
        "response": "Insufficient funds available to deposit."
    })";

    RestApiMessageParser parser;
    auto resp = parser.parseSimpleResponse(kErr);

    EXPECT_EQ(resp.status, "err");
    ASSERT_TRUE(resp.error.has_value());
    EXPECT_EQ(*resp.error, "Insufficient funds available to deposit.");
}
