#include <gtest/gtest.h>

#include "hyperliquid/config/Config.h"
#include "messages/ExchangeRequestBuilder.h"
#include "signing/Signing.h"

using namespace hyperliquid;

static const std::string kDummyPrivateKey =
    "0123456789012345678901234567890123456789012345678901234567890123";
static const std::string kDestination = "0x5e9ee1089755c3435139848e47e6635505d5a13a";
static const std::string kVaultAddress = "0x1719884eb866cb12b2287399b15f7db5e7d775ea";

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

// --- twapOrder / twapCancel (L1 actions) ---

TEST(TwapOrderBuilder, BodyShape)
{
    ExchangeRequestBuilder builder;

    TwapOrderRequest req;
    req.asset = "ETH";
    req.assetId = 4;
    req.isBuy = true;
    req.size = 0.5;
    req.reduceOnly = false;
    req.minutes = 10;
    req.randomize = true;

    auto body = builder.twapOrder(req);
    ASSERT_TRUE(body.contains("action"));
    const auto& action = body["action"];
    EXPECT_EQ(action["type"], "twapOrder");
    ASSERT_TRUE(action.contains("twap"));

    const auto& twap = action["twap"];
    EXPECT_EQ(twap["a"], 4);
    EXPECT_EQ(twap["b"], true);
    EXPECT_EQ(twap["s"], "0.5");
    EXPECT_EQ(twap["r"], false);
    EXPECT_EQ(twap["m"], 10);
    EXPECT_EQ(twap["t"], true);
}

TEST(TwapOrderBuilder, ActionIsSignable)
{
    ExchangeRequestBuilder builder;
    TwapOrderRequest req;
    req.asset = "ETH";
    req.assetId = 4;
    req.isBuy = true;
    req.size = 0.5;
    req.reduceOnly = false;
    req.minutes = 10;
    req.randomize = false;

    auto action = builder.twapOrder(req)["action"];
    auto sig = Signing::signL1Action(dummyWallet(), action, std::nullopt, 0, std::nullopt, false);
    EXPECT_FALSE(sig.r.empty());
    EXPECT_FALSE(sig.s.empty());
}

TEST(TwapCancelBuilder, BodyShape)
{
    ExchangeRequestBuilder builder;

    TwapCancelRequest req;
    req.asset = "ETH";
    req.assetId = 4;
    req.twapId = 77738308;

    auto body = builder.twapCancel(req);
    ASSERT_TRUE(body.contains("action"));
    const auto& action = body["action"];
    EXPECT_EQ(action["type"], "twapCancel");
    EXPECT_EQ(action["a"], 4);
    EXPECT_EQ(action["t"].get<uint64_t>(), 77738308ULL);
}

// --- vaultTransfer (L1 action) ---

TEST(VaultTransferBuilder, DepositBodyShape)
{
    ExchangeRequestBuilder builder;

    VaultTransferRequest req;
    req.vaultAddress = kVaultAddress;
    req.isDeposit = true;
    req.usd = 100.0;

    auto body = builder.vaultTransfer(req);
    ASSERT_TRUE(body.contains("action"));
    const auto& action = body["action"];
    EXPECT_EQ(action["type"], "vaultTransfer");
    EXPECT_EQ(action["vaultAddress"], kVaultAddress);
    EXPECT_EQ(action["isDeposit"], true);
    EXPECT_EQ(action["usd"].get<uint64_t>(), 100000000ULL);
}

TEST(VaultTransferBuilder, UsdScalesFractionalDollarsToRawUnits)
{
    ExchangeRequestBuilder builder;

    VaultTransferRequest req;
    req.vaultAddress = kVaultAddress;
    req.isDeposit = true;
    req.usd = 5.5;

    auto body = builder.vaultTransfer(req);
    EXPECT_EQ(body["action"]["usd"].get<uint64_t>(), 5500000ULL);
}

TEST(VaultTransferBuilder, ActionIsSignable)
{
    ExchangeRequestBuilder builder;
    VaultTransferRequest req;
    req.vaultAddress = kVaultAddress;
    req.isDeposit = false;
    req.usd = 50;

    auto action = builder.vaultTransfer(req)["action"];
    auto sig = Signing::signL1Action(dummyWallet(), action, std::nullopt, 0, std::nullopt, false);
    EXPECT_FALSE(sig.r.empty());
    EXPECT_FALSE(sig.s.empty());
}

// --- usdSend / withdraw3 (user-signed): cross-checked against the same reference
// signatures already verified in signing_test.cpp (SignUsdTransferAction /
// SignWithdrawFromBridgeAction), proving the builder's field names/formatting are
// byte-for-byte compatible with the production signing vectors. ---

TEST(UsdSendBuilder, BodyShapeAndReferenceSignature)
{
    ExchangeRequestBuilder builder;

    UsdSendRequest req;
    req.destination = kDestination;
    req.amount = 1;

    auto action = builder.usdSend(req)["action"];
    EXPECT_EQ(action["type"], "usdSend");
    EXPECT_EQ(action["destination"], kDestination);
    EXPECT_EQ(action["amount"], "1");

    action["hyperliquidChain"] = "Testnet";
    action["signatureChainId"] = "0x66eee";
    action["time"] = 1687816341423ULL;

    auto sig = Signing::signUserSignedAction(
        dummyWallet(), action,
        {
            {"hyperliquidChain", "string"},
            {"destination", "string"},
            {"amount", "string"},
            {"time", "uint64"},
        },
        "HyperliquidTransaction:UsdSend");

    EXPECT_EQ(sig.r, "0x637b37dd731507cdd24f46532ca8ba6eec616952c56218baeff04144e4a77073");
    EXPECT_EQ(sig.s, "0x11a6a24900e6e314136d2592e2f8d502cd89b7c15b198e1bee043c9589f9fad7");
    EXPECT_EQ(sig.v, 27);
}

TEST(Withdraw3Builder, BodyShapeAndReferenceSignature)
{
    ExchangeRequestBuilder builder;

    Withdraw3Request req;
    req.destination = kDestination;
    req.amount = 1;

    auto action = builder.withdraw3(req)["action"];
    EXPECT_EQ(action["type"], "withdraw3");
    EXPECT_EQ(action["destination"], kDestination);
    EXPECT_EQ(action["amount"], "1");

    action["hyperliquidChain"] = "Testnet";
    action["signatureChainId"] = "0x66eee";
    action["time"] = 1687816341423ULL;

    auto sig = Signing::signUserSignedAction(
        dummyWallet(), action,
        {
            {"hyperliquidChain", "string"},
            {"destination", "string"},
            {"amount", "string"},
            {"time", "uint64"},
        },
        "HyperliquidTransaction:Withdraw");

    EXPECT_EQ(sig.r, "0x8363524c799e90ce9bc41022f7c39b4e9bdba786e5f9c72b20e43e1462c37cf9");
    EXPECT_EQ(sig.s, "0x58b1411a775938b83e29182e8ef74975f9054c8e97ebf5ec2dc8d51bfc893881");
    EXPECT_EQ(sig.v, 28);
}

// --- prepareUserSignedActionBody round trip for all six user-signed actions:
// the dispatch table's per-type field list/primaryType is exercised end to end and
// cross-checked by independently recomputing the same signature from the observed
// nonce/action fields via the low-level Signing::signUserSignedAction. ---

TEST(PrepareUserSignedActionBody, UsdSendProducesFullySignedBody)
{
    ExchangeRequestBuilder builder;
    UsdSendRequest req;
    req.destination = kDestination;
    req.amount = 1;

    auto action = builder.usdSend(req)["action"];
    auto wallet = dummyWallet();
    auto config = testnetConfig(wallet);

    auto body = Signing::prepareUserSignedActionBody(config, RestEndpointType::UsdSend, action);

    ASSERT_TRUE(body.contains("action"));
    ASSERT_TRUE(body.contains("nonce"));
    ASSERT_TRUE(body.contains("signature"));

    const auto& signedAction = body["action"];
    EXPECT_EQ(signedAction["hyperliquidChain"], "Testnet");
    EXPECT_EQ(signedAction["signatureChainId"], "0x66eee");
    EXPECT_EQ(signedAction["time"].get<uint64_t>(), body["nonce"].get<uint64_t>());

    auto expectedSig = Signing::signUserSignedAction(
        wallet, signedAction,
        {
            {"hyperliquidChain", "string"},
            {"destination", "string"},
            {"amount", "string"},
            {"time", "uint64"},
        },
        "HyperliquidTransaction:UsdSend");

    EXPECT_EQ(body["signature"]["r"], expectedSig.r);
    EXPECT_EQ(body["signature"]["s"], expectedSig.s);
    EXPECT_EQ(body["signature"]["v"], expectedSig.v);
}

TEST(PrepareUserSignedActionBody, MissingWalletReturnsEmptyBody)
{
    ApiConfig config;
    config.env = Environment::Testnet;
    config.skipBuildingSymbolMap = true;

    ExchangeRequestBuilder builder;
    UsdSendRequest req;
    req.destination = kDestination;
    req.amount = 1;
    auto action = builder.usdSend(req)["action"];

    auto body = Signing::prepareUserSignedActionBody(config, RestEndpointType::UsdSend, action);
    EXPECT_TRUE(body.empty());
}

// --- spotSend (user-signed) ---

TEST(SpotSendBuilder, BodyShape)
{
    ExchangeRequestBuilder builder;

    SpotSendRequest req;
    req.destination = kDestination;
    req.token = "USDC:0xeb62eee3685fc4c43992febcd9e75443";
    req.amount = 1.5;

    auto action = builder.spotSend(req)["action"];
    EXPECT_EQ(action["type"], "spotSend");
    EXPECT_EQ(action["destination"], kDestination);
    EXPECT_EQ(action["token"], "USDC:0xeb62eee3685fc4c43992febcd9e75443");
    EXPECT_EQ(action["amount"], "1.5");
}

TEST(PrepareUserSignedActionBody, SpotSendRoundTrip)
{
    ExchangeRequestBuilder builder;
    SpotSendRequest req;
    req.destination = kDestination;
    req.token = "USDC:0xeb62eee3685fc4c43992febcd9e75443";
    req.amount = 1.5;

    auto action = builder.spotSend(req)["action"];
    auto wallet = dummyWallet();
    auto config = testnetConfig(wallet);

    auto body = Signing::prepareUserSignedActionBody(config, RestEndpointType::SpotSend, action);
    const auto& signedAction = body["action"];

    auto expectedSig = Signing::signUserSignedAction(
        wallet, signedAction,
        {
            {"hyperliquidChain", "string"},
            {"destination", "string"},
            {"token", "string"},
            {"amount", "string"},
            {"time", "uint64"},
        },
        "HyperliquidTransaction:SpotSend");

    EXPECT_EQ(body["signature"]["r"], expectedSig.r);
    EXPECT_EQ(body["signature"]["s"], expectedSig.s);
    EXPECT_EQ(body["signature"]["v"], expectedSig.v);
}

// --- usdClassTransfer (user-signed) ---

TEST(UsdClassTransferBuilder, BodyShape)
{
    ExchangeRequestBuilder builder;

    UsdClassTransferRequest req;
    req.amount = 10;
    req.toPerp = true;

    auto action = builder.usdClassTransfer(req)["action"];
    EXPECT_EQ(action["type"], "usdClassTransfer");
    EXPECT_EQ(action["amount"], "10");
    EXPECT_EQ(action["toPerp"], true);
}

TEST(PrepareUserSignedActionBody, UsdClassTransferUsesNonceField)
{
    ExchangeRequestBuilder builder;
    UsdClassTransferRequest req;
    req.amount = 10;
    req.toPerp = true;

    auto action = builder.usdClassTransfer(req)["action"];
    auto wallet = dummyWallet();
    auto config = testnetConfig(wallet);

    auto body = Signing::prepareUserSignedActionBody(config, RestEndpointType::UsdClassTransfer, action);
    const auto& signedAction = body["action"];

    // usdClassTransfer signs "nonce", not "time".
    EXPECT_FALSE(signedAction.contains("time"));
    ASSERT_TRUE(signedAction.contains("nonce"));
    EXPECT_EQ(signedAction["nonce"].get<uint64_t>(), body["nonce"].get<uint64_t>());

    auto expectedSig = Signing::signUserSignedAction(
        wallet, signedAction,
        {
            {"hyperliquidChain", "string"},
            {"amount", "string"},
            {"toPerp", "bool"},
            {"nonce", "uint64"},
        },
        "HyperliquidTransaction:UsdClassTransfer");

    EXPECT_EQ(body["signature"]["r"], expectedSig.r);
    EXPECT_EQ(body["signature"]["s"], expectedSig.s);
    EXPECT_EQ(body["signature"]["v"], expectedSig.v);
}

// --- sendAsset (user-signed) ---

TEST(SendAssetBuilder, BodyShape)
{
    ExchangeRequestBuilder builder;

    SendAssetRequest req;
    req.destination = kDestination;
    req.sourceDex = "";
    req.destinationDex = "test";
    req.token = "USDC:0xeb62eee3685fc4c43992febcd9e75443";
    req.amount = 2;
    req.fromSubAccount = "";

    auto action = builder.sendAsset(req)["action"];
    EXPECT_EQ(action["type"], "sendAsset");
    EXPECT_EQ(action["destination"], kDestination);
    EXPECT_EQ(action["sourceDex"], "");
    EXPECT_EQ(action["destinationDex"], "test");
    EXPECT_EQ(action["token"], "USDC:0xeb62eee3685fc4c43992febcd9e75443");
    EXPECT_EQ(action["amount"], "2");
    EXPECT_EQ(action["fromSubAccount"], "");
}

TEST(PrepareUserSignedActionBody, SendAssetRoundTrip)
{
    ExchangeRequestBuilder builder;
    SendAssetRequest req;
    req.destination = kDestination;
    req.sourceDex = "";
    req.destinationDex = "test";
    req.token = "USDC:0xeb62eee3685fc4c43992febcd9e75443";
    req.amount = 2;
    req.fromSubAccount = "";

    auto action = builder.sendAsset(req)["action"];
    auto wallet = dummyWallet();
    auto config = testnetConfig(wallet);

    auto body = Signing::prepareUserSignedActionBody(config, RestEndpointType::SendAsset, action);
    const auto& signedAction = body["action"];
    ASSERT_TRUE(signedAction.contains("nonce"));

    auto expectedSig = Signing::signUserSignedAction(
        wallet, signedAction,
        {
            {"hyperliquidChain", "string"},
            {"destination", "string"},
            {"sourceDex", "string"},
            {"destinationDex", "string"},
            {"token", "string"},
            {"amount", "string"},
            {"fromSubAccount", "string"},
            {"nonce", "uint64"},
        },
        "HyperliquidTransaction:SendAsset");

    EXPECT_EQ(body["signature"]["r"], expectedSig.r);
    EXPECT_EQ(body["signature"]["s"], expectedSig.s);
    EXPECT_EQ(body["signature"]["v"], expectedSig.v);
}

// --- approveBuilderFee (user-signed) ---

TEST(ApproveBuilderFeeBuilder, BodyShape)
{
    ExchangeRequestBuilder builder;

    ApproveBuilderFeeRequest req;
    req.builder = kDestination;
    req.maxFeeRate = "0.001%";

    auto action = builder.approveBuilderFee(req)["action"];
    EXPECT_EQ(action["type"], "approveBuilderFee");
    EXPECT_EQ(action["builder"], kDestination);
    EXPECT_EQ(action["maxFeeRate"], "0.001%");
}

TEST(PrepareUserSignedActionBody, ApproveBuilderFeeRoundTrip)
{
    ExchangeRequestBuilder builder;
    ApproveBuilderFeeRequest req;
    req.builder = kDestination;
    req.maxFeeRate = "0.001%";

    auto action = builder.approveBuilderFee(req)["action"];
    auto wallet = dummyWallet();
    auto config = testnetConfig(wallet);

    auto body = Signing::prepareUserSignedActionBody(config, RestEndpointType::ApproveBuilderFee, action);
    const auto& signedAction = body["action"];
    ASSERT_TRUE(signedAction.contains("nonce"));

    auto expectedSig = Signing::signUserSignedAction(
        wallet, signedAction,
        {
            {"hyperliquidChain", "string"},
            {"maxFeeRate", "string"},
            {"builder", "address"},
            {"nonce", "uint64"},
        },
        "HyperliquidTransaction:ApproveBuilderFee");

    EXPECT_EQ(body["signature"]["r"], expectedSig.r);
    EXPECT_EQ(body["signature"]["s"], expectedSig.s);
    EXPECT_EQ(body["signature"]["v"], expectedSig.v);
}

// --- userSetAbstraction (user-signed): request-builder and prepareUserSignedActionBody
// shape verified against the confirmed schema/EIP-712 types in the nktkas/hyperliquid TS SDK
// (src/api/exchange/_methods/userSetAbstraction.ts) - a synthetic payload, since no real
// signature vector for this action was published there. ---

static const std::string kUserSetAbstractionUser = "0x3b4d2cc2e144a0044002506c8b44508e9ace82e9";

TEST(UserSetAbstractionBuilder, BodyShapeDisabled)
{
    ExchangeRequestBuilder builder;

    UserSetAbstractionRequest req;
    req.user = kUserSetAbstractionUser;
    req.abstraction = AbstractionMode::Disabled;

    auto action = builder.userSetAbstraction(req)["action"];
    EXPECT_EQ(action["type"], "userSetAbstraction");
    EXPECT_EQ(action["user"], kUserSetAbstractionUser);
    EXPECT_EQ(action["abstraction"], "disabled");
}

TEST(UserSetAbstractionBuilder, BodyShapeUnifiedAccount)
{
    ExchangeRequestBuilder builder;

    UserSetAbstractionRequest req;
    req.user = kUserSetAbstractionUser;
    req.abstraction = AbstractionMode::UnifiedAccount;

    auto action = builder.userSetAbstraction(req)["action"];
    EXPECT_EQ(action["abstraction"], "unifiedAccount");
}

TEST(UserSetAbstractionBuilder, BodyShapePortfolioMargin)
{
    ExchangeRequestBuilder builder;

    UserSetAbstractionRequest req;
    req.user = kUserSetAbstractionUser;
    req.abstraction = AbstractionMode::PortfolioMargin;

    auto action = builder.userSetAbstraction(req)["action"];
    EXPECT_EQ(action["abstraction"], "portfolioMargin");
}

TEST(PrepareUserSignedActionBody, UserSetAbstractionRoundTrip)
{
    ExchangeRequestBuilder builder;
    UserSetAbstractionRequest req;
    req.user = kUserSetAbstractionUser;
    req.abstraction = AbstractionMode::UnifiedAccount;

    auto action = builder.userSetAbstraction(req)["action"];
    auto wallet = dummyWallet();
    auto config = testnetConfig(wallet);

    auto body = Signing::prepareUserSignedActionBody(config, RestEndpointType::UserSetAbstraction, action);
    const auto& signedAction = body["action"];
    ASSERT_TRUE(signedAction.contains("nonce"));
    EXPECT_EQ(signedAction["user"], kUserSetAbstractionUser);
    EXPECT_EQ(signedAction["abstraction"], "unifiedAccount");
    EXPECT_EQ(signedAction["hyperliquidChain"], "Testnet");
    EXPECT_EQ(signedAction["signatureChainId"], "0x66eee");

    auto expectedSig = Signing::signUserSignedAction(
        wallet, signedAction,
        {
            {"hyperliquidChain", "string"},
            {"user", "address"},
            {"abstraction", "string"},
            {"nonce", "uint64"},
        },
        "HyperliquidTransaction:UserSetAbstraction");

    EXPECT_EQ(body["signature"]["r"], expectedSig.r);
    EXPECT_EQ(body["signature"]["s"], expectedSig.s);
    EXPECT_EQ(body["signature"]["v"], expectedSig.v);
}

// --- Generic ack / TWAP response parsing ---

#include "hyperliquid/rest/RestApiMessageParser.h"

TEST(SimpleResponseParsing, VaultTransferSuccess)
{
    static const std::string kOk = R"({"status":"ok","response":{"type":"default"}})";
    RestApiMessageParser parser;
    auto resp = parser.parseSimpleResponse(kOk);
    EXPECT_EQ(resp.status, "ok");
    EXPECT_FALSE(resp.error.has_value());
}

// Real testnet response: a signed userSetAbstraction(Disabled) request against an account
// with no open positions/orders - matches the shape documented for UserSetAbstractionResponse
// in the nktkas/hyperliquid TS SDK exactly.
TEST(SimpleResponseParsing, UserSetAbstractionSuccess)
{
    static const std::string kOk = R"({"status":"ok","response":{"type":"default"}})";
    RestApiMessageParser parser;
    auto resp = parser.parseSimpleResponse(kOk);
    EXPECT_EQ(resp.status, "ok");
    EXPECT_FALSE(resp.error.has_value());
}

// Synthetic - modeled on the portfolio-margin-eligibility rejection asserted in that SDK's
// tests/api/exchange/userSetAbstraction.test.ts ("Portfolio margin requires account value of
// $10000 or total volume of $5000000.").
TEST(SimpleResponseParsing, UserSetAbstractionPortfolioMarginRejected)
{
    static const std::string kErr =
        R"({"status":"err","response":"Portfolio margin requires account value of $10000 or total volume of $5000000."})";
    RestApiMessageParser parser;
    auto resp = parser.parseSimpleResponse(kErr);
    EXPECT_EQ(resp.status, "err");
    ASSERT_TRUE(resp.error.has_value());
    EXPECT_EQ(*resp.error, "Portfolio margin requires account value of $10000 or total volume of $5000000.");
}

// Real testnet response: a signed userSetAbstraction(Disabled) request against an account
// already in UnifiedAccount mode with open positions - confirms the request/signing/response
// round-trip end-to-end.
TEST(SimpleResponseParsing, UserSetAbstractionDisableRejectedWithOpenPositions)
{
    static const std::string kErr =
        R"({"status":"err","response":"Cannot disable unified account with open positions, open orders, or active TWAP orders"})";
    RestApiMessageParser parser;
    auto resp = parser.parseSimpleResponse(kErr);
    EXPECT_EQ(resp.status, "err");
    ASSERT_TRUE(resp.error.has_value());
    EXPECT_EQ(*resp.error, "Cannot disable unified account with open positions, open orders, or active TWAP orders");
}

TEST(TwapOrderResponseParsing, RunningResponse)
{
    static const std::string kRunning = R"({
        "status":"ok",
        "response":{
            "type":"twapOrder",
            "data":{"status":{"running":{"twapId":77738308}}}
        }
    })";

    RestApiMessageParser parser;
    auto resp = parser.parseTwapOrder(kRunning);
    EXPECT_EQ(resp.status, "ok");
    EXPECT_EQ(resp.type, "twapOrder");
    ASSERT_TRUE(resp.twapId.has_value());
    EXPECT_EQ(*resp.twapId, 77738308ULL);
    EXPECT_FALSE(resp.error.has_value());
}

TEST(TwapOrderResponseParsing, ErrorResponse)
{
    static const std::string kErr = R"json({
        "status":"ok",
        "response":{
            "type":"twapOrder",
            "data":{"status":{"error":"Invalid TWAP duration: 1 min(s)"}}
        }
    })json";

    RestApiMessageParser parser;
    auto resp = parser.parseTwapOrder(kErr);
    EXPECT_EQ(resp.status, "ok");
    EXPECT_FALSE(resp.twapId.has_value());
    ASSERT_TRUE(resp.error.has_value());
    EXPECT_EQ(*resp.error, "Invalid TWAP duration: 1 min(s)");
}

TEST(TwapCancelResponseParsing, SuccessResponse)
{
    static const std::string kOk = R"({
        "status":"ok",
        "response":{
            "type":"twapCancel",
            "data":{"status":"success"}
        }
    })";

    RestApiMessageParser parser;
    auto resp = parser.parseTwapCancel(kOk);
    EXPECT_EQ(resp.status, "ok");
    ASSERT_TRUE(resp.success.has_value());
    EXPECT_EQ(*resp.success, "success");
    EXPECT_FALSE(resp.error.has_value());
}

TEST(TwapCancelResponseParsing, ErrorResponse)
{
    static const std::string kErr = R"({"status":"err","response":"twap not found"})";

    RestApiMessageParser parser;
    auto resp = parser.parseTwapCancel(kErr);
    EXPECT_EQ(resp.status, "err");
    ASSERT_TRUE(resp.error.has_value());
    EXPECT_EQ(*resp.error, "twap not found");
}
