#include <gtest/gtest.h>

#include "hyperliquid/config/Config.h"
#include "messages/ExchangeRequestBuilder.h"
#include "signing/Signing.h"
#include "signing/SigningHelpers.h"

using namespace hyperliquid;

static const std::string kDummyPrivateKey =
    "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
static const std::string kDummyAgentAddress = "0x0000000000000000000000000000000000000001";

static ApiConfig testConfig(Environment env)
{
    ApiConfig config;
    config.env = env;
    config.wallet = Wallet{"", kDummyPrivateKey};
    config.skipBuildingSymbolMap = true;
    return config;
}

TEST(ExchangeRequestBuilderTest, ApproveAgentWithNameBodyShape)
{
    ExchangeRequestBuilder builder;

    ApproveAgentRequest req;
    req.agentAddress = kDummyAgentAddress;
    req.agentName = "my-agent";

    auto body = builder.approveAgent(req);
    ASSERT_TRUE(body.contains("action"));

    const auto& action = body["action"];
    EXPECT_EQ(action["type"], "approveAgent");
    EXPECT_EQ(action["agentAddress"], kDummyAgentAddress);
    ASSERT_TRUE(action.contains("agentName"));
    EXPECT_EQ(action["agentName"], "my-agent");
}

TEST(ExchangeRequestBuilderTest, ApproveAgentUnnamedOmitsAgentNameField)
{
    ExchangeRequestBuilder builder;

    ApproveAgentRequest req;
    req.agentAddress = kDummyAgentAddress;

    auto body = builder.approveAgent(req);
    const auto& action = body["action"];
    EXPECT_EQ(action["type"], "approveAgent");
    EXPECT_FALSE(action.contains("agentName"));
}

TEST(ApproveAgentSigning, PreparedBodyShapeNamedTestnet)
{
    auto config = testConfig(Environment::Testnet);

    auto body = Signing::prepareApproveAgentBody(config, kDummyAgentAddress, std::string("my-agent"));

    ASSERT_TRUE(body.contains("action"));
    ASSERT_TRUE(body.contains("nonce"));
    ASSERT_TRUE(body.contains("signature"));

    const auto& action = body["action"];
    EXPECT_EQ(action["type"], "approveAgent");
    EXPECT_EQ(action["agentAddress"], kDummyAgentAddress);
    EXPECT_EQ(action["agentName"], "my-agent");
    EXPECT_EQ(action["signatureChainId"], "0x66eee");
    EXPECT_EQ(action["hyperliquidChain"], "Testnet");
    ASSERT_TRUE(action.contains("nonce"));
    EXPECT_EQ(action["nonce"].get<uint64_t>(), body["nonce"].get<uint64_t>());

    const auto& signature = body["signature"];
    EXPECT_FALSE(signature["r"].get<std::string>().empty());
    EXPECT_FALSE(signature["s"].get<std::string>().empty());
}

TEST(ApproveAgentSigning, PreparedBodyUsesMainnetChainLabel)
{
    auto config = testConfig(Environment::Mainnet);

    auto body = Signing::prepareApproveAgentBody(config, kDummyAgentAddress, std::nullopt);

    EXPECT_EQ(body["action"]["hyperliquidChain"], "Mainnet");
}

TEST(ApproveAgentSigning, UnnamedApprovalOmitsAgentNameFromOutgoingBodyButSignsWithEmptyString)
{
    auto config = testConfig(Environment::Testnet);

    auto body = Signing::prepareApproveAgentBody(config, kDummyAgentAddress, std::nullopt);

    EXPECT_FALSE(body["action"].contains("agentName"));

    uint64_t nonce = body["nonce"].get<uint64_t>();

    nlohmann::ordered_json signedAction;
    signedAction["type"] = "approveAgent";
    signedAction["agentAddress"] = kDummyAgentAddress;
    signedAction["agentName"] = "";
    signedAction["nonce"] = nonce;
    signedAction["signatureChainId"] = "0x66eee";
    signedAction["hyperliquidChain"] = "Testnet";

    auto expectedSig = Signing::signUserSignedAction(
        config.wallet.value(), signedAction,
        {
            {"hyperliquidChain", "string"},
            {"agentAddress", "address"},
            {"agentName", "string"},
            {"nonce", "uint64"},
        },
        "HyperliquidTransaction:ApproveAgent", false);

    EXPECT_EQ(body["signature"]["r"], expectedSig.r);
    EXPECT_EQ(body["signature"]["s"], expectedSig.s);
    EXPECT_EQ(body["signature"]["v"], expectedSig.v);
}

TEST(ApproveAgentSigning, MissingWalletReturnsBodyWithoutSignature)
{
    ApiConfig config;
    config.env = Environment::Testnet;
    config.skipBuildingSymbolMap = true;

    auto body = Signing::prepareApproveAgentBody(config, kDummyAgentAddress, std::nullopt);

    EXPECT_FALSE(body.contains("signature"));
    EXPECT_FALSE(body.contains("action"));
}

TEST(SigningHelpersTest, GeneratedKeypairAddressRoundTrips)
{
    auto privateKey = SigningHelpers::generatePrivateKeyHex();
    auto address1 = SigningHelpers::privateKeyToAddress(privateKey);
    auto address2 = SigningHelpers::privateKeyToAddress(privateKey);

    EXPECT_EQ(address1, address2);
    EXPECT_EQ(address1.size(), 42u);
    EXPECT_EQ(address1.substr(0, 2), "0x");
}

TEST(SigningHelpersTest, GeneratedKeypairsAreDistinct)
{
    auto key1 = SigningHelpers::generatePrivateKeyHex();
    auto key2 = SigningHelpers::generatePrivateKeyHex();

    EXPECT_NE(key1, key2);
}
