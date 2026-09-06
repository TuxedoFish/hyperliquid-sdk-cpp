#include <gtest/gtest.h>

#include "hyperliquid/config/Config.h"
#include "hyperliquid/rest/RestApiMessageParser.h"
#include "messages/ExchangeRequestBuilder.h"
#include "signing/Signing.h"

using namespace hyperliquid;

// Request shape and reference address lifted from nktkas/hyperliquid's
// tests/api/exchange/userDexAbstraction.test.ts - synthetic (never sent to a live server).
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

TEST(UserDexAbstractionBuilder, EnabledBodyShape)
{
    ExchangeRequestBuilder builder;

    UserDexAbstractionRequest req;
    req.user = kUser;
    req.enabled = true;

    auto action = builder.userDexAbstraction(req)["action"];
    EXPECT_EQ(action["type"], "userDexAbstraction");
    EXPECT_EQ(action["user"], kUser);
    EXPECT_EQ(action["enabled"], true);
}

TEST(UserDexAbstractionBuilder, DisabledBodyShape)
{
    ExchangeRequestBuilder builder;

    UserDexAbstractionRequest req;
    req.user = kUser;
    req.enabled = false;

    auto action = builder.userDexAbstraction(req)["action"];
    EXPECT_EQ(action["type"], "userDexAbstraction");
    EXPECT_EQ(action["enabled"], false);
}

TEST(PrepareUserSignedActionBody, UserDexAbstractionRoundTrip)
{
    ExchangeRequestBuilder builder;
    UserDexAbstractionRequest req;
    req.user = kUser;
    req.enabled = true;

    auto action = builder.userDexAbstraction(req)["action"];
    auto wallet = dummyWallet();
    auto config = testnetConfig(wallet);

    auto body = Signing::prepareUserSignedActionBody(config, RestEndpointType::UserDexAbstraction, action);

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
        "HyperliquidTransaction:UserDexAbstraction");

    EXPECT_EQ(body["signature"]["r"], expectedSig.r);
    EXPECT_EQ(body["signature"]["s"], expectedSig.s);
    EXPECT_EQ(body["signature"]["v"], expectedSig.v);
}

TEST(PrepareUserSignedActionBody, UserDexAbstractionUsesMainnetChainLabel)
{
    ExchangeRequestBuilder builder;
    UserDexAbstractionRequest req;
    req.user = kUser;
    req.enabled = false;

    auto action = builder.userDexAbstraction(req)["action"];
    auto wallet = dummyWallet();
    ApiConfig config;
    config.env = Environment::Mainnet;
    config.wallet = wallet;
    config.skipBuildingSymbolMap = true;

    auto body = Signing::prepareUserSignedActionBody(config, RestEndpointType::UserDexAbstraction, action);
    EXPECT_EQ(body["action"]["hyperliquidChain"], "Mainnet");
}

TEST(PrepareUserSignedActionBody, UserDexAbstractionMissingWalletReturnsEmptyBody)
{
    ApiConfig config;
    config.env = Environment::Testnet;
    config.skipBuildingSymbolMap = true;

    ExchangeRequestBuilder builder;
    UserDexAbstractionRequest req;
    req.user = kUser;
    req.enabled = true;
    auto action = builder.userDexAbstraction(req)["action"];

    auto body = Signing::prepareUserSignedActionBody(config, RestEndpointType::UserDexAbstraction, action);
    EXPECT_TRUE(body.empty());
}

// Synthetic response fixtures matching the TS SDK's UserDexAbstractionResponse shape
// (status:"ok"/response.type:"default" on success, status:"err"/string response on error) - the
// same shape already exercised generically by SimpleResponseParsing.VaultTransferSuccess.

TEST(UserDexAbstractionResponseParsing, SuccessResponse)
{
    static const std::string kOk = R"({"status":"ok","response":{"type":"default"}})";
    RestApiMessageParser parser;
    auto resp = parser.parseSimpleResponse(kOk);
    EXPECT_EQ(resp.status, "ok");
    EXPECT_FALSE(resp.error.has_value());
}

TEST(UserDexAbstractionResponseParsing, ErrorResponse)
{
    static const std::string kErr =
        R"({"status":"err","response":"Sub-account 0xcb3f0bd249a89e45e86a44bcfc7113e4ffe84cd1 is not registered to caller"})";
    RestApiMessageParser parser;
    auto resp = parser.parseSimpleResponse(kErr);
    EXPECT_EQ(resp.status, "err");
    ASSERT_TRUE(resp.error.has_value());
    EXPECT_EQ(*resp.error, "Sub-account 0xcb3f0bd249a89e45e86a44bcfc7113e4ffe84cd1 is not registered to caller");
}
