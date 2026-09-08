#include <gtest/gtest.h>

#include "hyperliquid/config/Config.h"
#include "hyperliquid/rest/RestApiMessageParser.h"
#include "messages/ExchangeRequestBuilder.h"
#include "signing/Signing.h"

using namespace hyperliquid;

// Request shape (action.type/user/enabled fields, EIP-712 user-signed action) cross-checked
// against nktkas/hyperliquid's src/api/exchange/_methods/userPortfolioMargin.ts, which mirrors
// userDexAbstraction's field set exactly (user, enabled) - synthetic wallet/address below, never
// sent to a live server.
static const std::string kDummyPrivateKey =
    "0123456789012345678901234567890123456789012345678901234567890123";
static const std::string kUser = "0xcb3f0bd249a89e45e86a44bcfc7113e4ffe84cd1";

static Wallet dummyWallet()
{
    return Wallet{"", kDummyPrivateKey};
}

static ApiConfig testnetConfig(const Wallet& wallet)
{
    ApiConfig config;
    config.env = Environment::Testnet;
    config.wallet = wallet;
    config.skipBuildingSymbolMap = true;
    return config;
}

TEST(UserPortfolioMarginBuilder, EnabledBodyShape)
{
    ExchangeRequestBuilder builder;

    UserPortfolioMarginRequest req;
    req.user = kUser;
    req.enabled = true;

    auto action = builder.userPortfolioMargin(req)["action"];
    EXPECT_EQ(action["type"], "userPortfolioMargin");
    EXPECT_EQ(action["user"], kUser);
    EXPECT_EQ(action["enabled"], true);
}

TEST(UserPortfolioMarginBuilder, DisabledBodyShape)
{
    ExchangeRequestBuilder builder;

    UserPortfolioMarginRequest req;
    req.user = kUser;
    req.enabled = false;

    auto action = builder.userPortfolioMargin(req)["action"];
    EXPECT_EQ(action["type"], "userPortfolioMargin");
    EXPECT_EQ(action["enabled"], false);
}

TEST(PrepareUserSignedActionBody, UserPortfolioMarginRoundTrip)
{
    ExchangeRequestBuilder builder;
    UserPortfolioMarginRequest req;
    req.user = kUser;
    req.enabled = true;

    auto action = builder.userPortfolioMargin(req)["action"];
    auto wallet = dummyWallet();
    auto config = testnetConfig(wallet);

    auto body = Signing::prepareUserSignedActionBody(config, RestEndpointType::UserPortfolioMargin, action);

    ASSERT_TRUE(body.contains("action"));
    ASSERT_TRUE(body.contains("nonce"));
    ASSERT_TRUE(body.contains("signature"));

    const auto& signedAction = body["action"];
    EXPECT_EQ(signedAction["hyperliquidChain"], "Testnet");
    EXPECT_EQ(signedAction["signatureChainId"], "0x66eee");
    EXPECT_FALSE(signedAction.contains("time"));
    ASSERT_TRUE(signedAction.contains("nonce"));
    EXPECT_EQ(signedAction["nonce"].get<uint64_t>(), body["nonce"].get<uint64_t>());

    auto expectedSig = Signing::signUserSignedAction(
        wallet, signedAction,
        {
            {"hyperliquidChain", "string"},
            {"user", "address"},
            {"enabled", "bool"},
            {"nonce", "uint64"},
        },
        "HyperliquidTransaction:UserPortfolioMargin");

    EXPECT_EQ(body["signature"]["r"], expectedSig.r);
    EXPECT_EQ(body["signature"]["s"], expectedSig.s);
    EXPECT_EQ(body["signature"]["v"], expectedSig.v);
}

TEST(PrepareUserSignedActionBody, UserPortfolioMarginUsesMainnetChainLabel)
{
    ExchangeRequestBuilder builder;
    UserPortfolioMarginRequest req;
    req.user = kUser;
    req.enabled = false;

    auto action = builder.userPortfolioMargin(req)["action"];
    auto wallet = dummyWallet();
    ApiConfig config;
    config.env = Environment::Mainnet;
    config.wallet = wallet;
    config.skipBuildingSymbolMap = true;

    auto body = Signing::prepareUserSignedActionBody(config, RestEndpointType::UserPortfolioMargin, action);
    EXPECT_EQ(body["action"]["hyperliquidChain"], "Mainnet");
}

TEST(PrepareUserSignedActionBody, UserPortfolioMarginMissingWalletThrows)
{
    ApiConfig config;
    config.env = Environment::Testnet;
    config.skipBuildingSymbolMap = true;

    ExchangeRequestBuilder builder;
    UserPortfolioMarginRequest req;
    req.user = kUser;
    req.enabled = true;
    auto action = builder.userPortfolioMargin(req)["action"];

    EXPECT_THROW(
        Signing::prepareUserSignedActionBody(config, RestEndpointType::UserPortfolioMargin, action),
        std::invalid_argument);
}

// Both fixtures below are real testnet captures (POST /exchange, userPortfolioMargin), not
// inferred from the TS SDK: enabling failed with a real business-rule error on this test wallet
// (insufficient account value/volume), and disabling (already-off, so a no-op) succeeded with
// the generic {status:"ok", response:{type:"default"}} shape shared by every other simple
// exchange action.

TEST(UserPortfolioMarginResponseParsing, SuccessResponse)
{
    static const std::string kOk = R"({"status":"ok","response":{"type":"default"}})";
    RestApiMessageParser parser;
    auto resp = parser.parseSimpleResponse(kOk);
    EXPECT_EQ(resp.status, "ok");
    EXPECT_FALSE(resp.error.has_value());
}

TEST(UserPortfolioMarginResponseParsing, ErrorResponse)
{
    static const std::string kErr =
        R"({"status":"err","response":"Portfolio margin requires account value of $10000 or total volume of $5000000."})";
    RestApiMessageParser parser;
    auto resp = parser.parseSimpleResponse(kErr);
    EXPECT_EQ(resp.status, "err");
    ASSERT_TRUE(resp.error.has_value());
    EXPECT_EQ(*resp.error, "Portfolio margin requires account value of $10000 or total volume of $5000000.");
}
