#include <gtest/gtest.h>

#include "hyperliquid/config/Config.h"
#include "signing/Signing.h"
#include "signing/SigningHelpers.h"

#include <nlohmann/json.hpp>

using namespace hyperliquid;

static const std::string TEST_PRIVATE_KEY = "0123456789012345678901234567890123456789012345678901234567890123";

// --- hexToBytes ---

TEST(SigningHelpersHex, StripsOxPrefix)
{
    auto withPrefix = SigningHelpers::hexToBytes("0xabcd");
    auto withoutPrefix = SigningHelpers::hexToBytes("abcd");
    EXPECT_EQ(withPrefix, withoutPrefix);
    ASSERT_EQ(withPrefix.size(), 2u);
    EXPECT_EQ(withPrefix[0], 0xab);
    EXPECT_EQ(withPrefix[1], 0xcd);
}

TEST(SigningHelpersHex, OddLengthIsLeftPaddedWithZero)
{
    // "abc" -> "0abc" -> {0x0a, 0xbc}
    auto bytes = SigningHelpers::hexToBytes("abc");
    ASSERT_EQ(bytes.size(), 2u);
    EXPECT_EQ(bytes[0], 0x0a);
    EXPECT_EQ(bytes[1], 0xbc);
}

TEST(SigningHelpersHex, UppercaseDigitsAccepted)
{
    auto lower = SigningHelpers::hexToBytes("0xDEADBEEF");
    auto upper = SigningHelpers::hexToBytes("0xdeadbeef");
    EXPECT_EQ(lower, upper);
}

TEST(SigningHelpersHex, InvalidCharacterThrows)
{
    EXPECT_THROW(SigningHelpers::hexToBytes("0xzz"), std::invalid_argument);
}

TEST(SigningHelpersHex, ToHexStripsLeadingZeroBytesButKeepsAtLeastOneDigit)
{
    uint8_t allZero[4] = {0x00, 0x00, 0x00, 0x00};
    EXPECT_EQ(SigningHelpers::toHex(allZero, 4), "0x0");

    uint8_t leadingZero[2] = {0x00, 0x0f};
    EXPECT_EQ(SigningHelpers::toHex(leadingZero, 2), "0xf");
}

TEST(SigningHelpersHex, ToHexPaddedPreservesLeadingZeroBytes)
{
    uint8_t leadingZero[2] = {0x00, 0x0f};
    EXPECT_EQ(SigningHelpers::toHexPadded(leadingZero, 2), "0x000f");
}

// --- encodeUint256 / encodeAddress ---

TEST(SigningHelpersEncode, EncodeUint256IsBigEndianRightAligned)
{
    auto encoded = SigningHelpers::encodeUint256(0x0102030405060708ULL);
    for (int idx = 0; idx < 24; idx++)
        EXPECT_EQ(encoded[idx], 0) << "byte " << idx;
    EXPECT_EQ(encoded[24], 0x01);
    EXPECT_EQ(encoded[31], 0x08);
}

TEST(SigningHelpersEncode, EncodeUint256Zero)
{
    auto encoded = SigningHelpers::encodeUint256(0);
    for (auto byte : encoded)
        EXPECT_EQ(byte, 0);
}

TEST(SigningHelpersEncode, EncodeUint256MaxValue)
{
    auto encoded = SigningHelpers::encodeUint256(UINT64_MAX);
    for (int idx = 0; idx < 24; idx++)
        EXPECT_EQ(encoded[idx], 0);
    for (int idx = 24; idx < 32; idx++)
        EXPECT_EQ(encoded[idx], 0xFF);
}

TEST(SigningHelpersEncode, EncodeAddressRightPadsIntoLast20Bytes)
{
    std::vector<uint8_t> address(20, 0xAB);
    auto encoded = SigningHelpers::encodeAddress(address);
    for (int idx = 0; idx < 12; idx++)
        EXPECT_EQ(encoded[idx], 0);
    for (int idx = 12; idx < 32; idx++)
        EXPECT_EQ(encoded[idx], 0xAB);
}

TEST(SigningHelpersEncode, EncodeAddressWithWrongLengthReturnsAllZero)
{
    // Documents current behavior: malformed (non-20-byte) input is silently
    // zero-filled rather than throwing.
    std::vector<uint8_t> tooShort(10, 0xAB);
    auto encoded = SigningHelpers::encodeAddress(tooShort);
    for (auto byte : encoded)
        EXPECT_EQ(byte, 0);
}

// --- actionHash: vaultAddress / expiresAfter edge cases ---

static nlohmann::ordered_json dummyAction()
{
    nlohmann::ordered_json action;
    action["type"] = "dummy";
    action["num"] = 1000;
    return action;
}

TEST(SigningHelpersActionHash, DeterministicForSameInputs)
{
    auto action = dummyAction();
    auto first = SigningHelpers::actionHash(action, std::nullopt, 7, std::nullopt);
    auto second = SigningHelpers::actionHash(action, std::nullopt, 7, std::nullopt);
    EXPECT_EQ(first, second);
}

TEST(SigningHelpersActionHash, VaultAddressChangesHash)
{
    auto action = dummyAction();
    auto withoutVault = SigningHelpers::actionHash(action, std::nullopt, 0, std::nullopt);
    auto withVault = SigningHelpers::actionHash(
        action, std::string("0x1719884eb866cb12b2287399b15f7db5e7d775ea"), 0, std::nullopt);
    EXPECT_NE(withoutVault, withVault);
}

TEST(SigningHelpersActionHash, DifferentVaultAddressesProduceDifferentHashes)
{
    auto action = dummyAction();
    auto vaultA = SigningHelpers::actionHash(
        action, std::string("0x1719884eb866cb12b2287399b15f7db5e7d775ea"), 0, std::nullopt);
    auto vaultB = SigningHelpers::actionHash(
        action, std::string("0x5e9ee1089755c3435139848e47e6635505d5a13a"), 0, std::nullopt);
    EXPECT_NE(vaultA, vaultB);
}

TEST(SigningHelpersActionHash, ExpiresAfterChangesHash)
{
    auto action = dummyAction();
    auto withoutExpiry = SigningHelpers::actionHash(action, std::nullopt, 0, std::nullopt);
    auto withExpiry = SigningHelpers::actionHash(action, std::nullopt, 0, uint64_t{123456789});
    EXPECT_NE(withoutExpiry, withExpiry);
}

TEST(SigningHelpersActionHash, DifferentExpiresAfterValuesProduceDifferentHashes)
{
    auto action = dummyAction();
    auto expiryA = SigningHelpers::actionHash(action, std::nullopt, 0, uint64_t{1});
    auto expiryB = SigningHelpers::actionHash(action, std::nullopt, 0, uint64_t{2});
    EXPECT_NE(expiryA, expiryB);
}

TEST(SigningHelpersActionHash, VaultAndExpiresAfterCombinedChangeHash)
{
    auto action = dummyAction();
    auto plain = SigningHelpers::actionHash(action, std::nullopt, 0, std::nullopt);
    auto vaultOnly = SigningHelpers::actionHash(
        action, std::string("0x1719884eb866cb12b2287399b15f7db5e7d775ea"), 0, std::nullopt);
    auto expiryOnly = SigningHelpers::actionHash(action, std::nullopt, 0, uint64_t{123456789});
    auto both = SigningHelpers::actionHash(
        action, std::string("0x1719884eb866cb12b2287399b15f7db5e7d775ea"), 0, uint64_t{123456789});

    EXPECT_NE(both, plain);
    EXPECT_NE(both, vaultOnly);
    EXPECT_NE(both, expiryOnly);
}

// --- Signing::signL1Action: expiresAfter / vault+expiresAfter combined ---

TEST(SigningExpiresAfter, ChangesSignatureVersusNoExpiry)
{
    Wallet wallet{"", TEST_PRIVATE_KEY};
    auto action = dummyAction();

    auto withoutExpiry = Signing::signL1Action(wallet, action, std::nullopt, 0, std::nullopt, true);
    auto withExpiry = Signing::signL1Action(wallet, action, std::nullopt, 0, uint64_t{123456789}, true);

    EXPECT_NE(withoutExpiry.r, withExpiry.r);
}

TEST(SigningExpiresAfter, DeterministicAcrossRepeatedCalls)
{
    Wallet wallet{"", TEST_PRIVATE_KEY};
    auto action = dummyAction();

    auto first = Signing::signL1Action(wallet, action, std::nullopt, 0, uint64_t{42}, true);
    auto second = Signing::signL1Action(wallet, action, std::nullopt, 0, uint64_t{42}, true);

    EXPECT_EQ(first.r, second.r);
    EXPECT_EQ(first.s, second.s);
    EXPECT_EQ(first.v, second.v);
}

TEST(SigningExpiresAfter, VaultAndExpiresAfterCombinedIsSignable)
{
    Wallet wallet{"", TEST_PRIVATE_KEY};
    auto action = dummyAction();
    std::string vaultAddress = "0x1719884eb866cb12b2287399b15f7db5e7d775ea";

    auto sig = Signing::signL1Action(wallet, action, vaultAddress, 0, uint64_t{123456789}, true);
    EXPECT_FALSE(sig.r.empty());
    EXPECT_FALSE(sig.s.empty());
    EXPECT_TRUE(sig.v == 27 || sig.v == 28);

    // Sanity: dropping either input changes the resulting signature.
    auto vaultOnly = Signing::signL1Action(wallet, action, vaultAddress, 0, std::nullopt, true);
    auto expiryOnly = Signing::signL1Action(wallet, action, std::nullopt, 0, uint64_t{123456789}, true);
    EXPECT_NE(sig.r, vaultOnly.r);
    EXPECT_NE(sig.r, expiryOnly.r);
}

// --- Signing::prepareBody ---

static nlohmann::ordered_json actionBody()
{
    nlohmann::ordered_json body;
    body["action"] = dummyAction();
    return body;
}

TEST(SigningPrepareBody, UnauthenticatedEndpointIsPassedThroughUnsigned)
{
    ApiConfig config;
    config.env = Environment::Mainnet;
    config.wallet = Wallet{"", TEST_PRIVATE_KEY};

    auto result = Signing::prepareBody(config, RestEndpointType::Meta, actionBody());

    EXPECT_FALSE(result.contains("nonce"));
    EXPECT_FALSE(result.contains("signature"));
}

TEST(SigningPrepareBody, UnauthenticatedEndpointStillMergesVaultAndExpiresAfter)
{
    ApiConfig config;
    config.env = Environment::Mainnet;
    config.wallet = Wallet{"", TEST_PRIVATE_KEY};

    auto result = Signing::prepareBody(
        config, RestEndpointType::Meta, actionBody(),
        std::string("0x1719884eb866cb12b2287399b15f7db5e7d775ea"), uint64_t{123456789});

    EXPECT_EQ(result["vaultAddress"], "0x1719884eb866cb12b2287399b15f7db5e7d775ea");
    EXPECT_EQ(result["expiresAfter"], 123456789ULL);
    EXPECT_FALSE(result.contains("signature"));
}

TEST(SigningPrepareBody, AuthenticatedEndpointWithoutWalletSkipsSigning)
{
    ApiConfig config;
    config.env = Environment::Mainnet;
    config.wallet = std::nullopt;

    auto result = Signing::prepareBody(config, RestEndpointType::PlaceOrder, actionBody());

    EXPECT_FALSE(result.contains("nonce"));
    EXPECT_FALSE(result.contains("signature"));
}

TEST(SigningPrepareBody, AuthenticatedEndpointWithWalletAddsNonceAndSignature)
{
    ApiConfig config;
    config.env = Environment::Mainnet;
    config.wallet = Wallet{"", TEST_PRIVATE_KEY};

    auto result = Signing::prepareBody(config, RestEndpointType::PlaceOrder, actionBody());

    ASSERT_TRUE(result.contains("nonce"));
    EXPECT_GT(result["nonce"].get<uint64_t>(), 0u);

    ASSERT_TRUE(result.contains("signature"));
    EXPECT_TRUE(result["signature"].contains("r"));
    EXPECT_TRUE(result["signature"].contains("s"));
    EXPECT_TRUE(result["signature"].contains("v"));
}

TEST(SigningPrepareBody, VaultAddressIsIncludedInSignedBody)
{
    ApiConfig config;
    config.env = Environment::Mainnet;
    config.wallet = Wallet{"", TEST_PRIVATE_KEY};

    auto result = Signing::prepareBody(
        config, RestEndpointType::PlaceOrder, actionBody(),
        std::string("0x1719884eb866cb12b2287399b15f7db5e7d775ea"));

    EXPECT_EQ(result["vaultAddress"], "0x1719884eb866cb12b2287399b15f7db5e7d775ea");
    ASSERT_TRUE(result.contains("signature"));
}

// --- Signing::prepareBodyForType (per-call vaultAddress vs ApiConfig::vaultAddress fallback) ---

TEST(SigningPrepareBodyForType, PerCallVaultAddressIsUsedWhenProvided)
{
    ApiConfig config;
    config.env = Environment::Mainnet;
    config.wallet = Wallet{"", TEST_PRIVATE_KEY};
    config.vaultAddress = "0x2222222222222222222222222222222222222222";

    auto result = Signing::prepareBodyForType(
        config, RestEndpointType::PlaceOrder, actionBody(),
        std::string("0x1719884eb866cb12b2287399b15f7db5e7d775ea"));

    EXPECT_EQ(result["vaultAddress"], "0x1719884eb866cb12b2287399b15f7db5e7d775ea");
}

TEST(SigningPrepareBodyForType, FallsBackToConfigVaultAddressWhenOmitted)
{
    ApiConfig config;
    config.env = Environment::Mainnet;
    config.wallet = Wallet{"", TEST_PRIVATE_KEY};
    config.vaultAddress = "0x2222222222222222222222222222222222222222";

    auto result = Signing::prepareBodyForType(config, RestEndpointType::PlaceOrder, actionBody());

    EXPECT_EQ(result["vaultAddress"], "0x2222222222222222222222222222222222222222");
}

TEST(SigningPrepareBodyForType, NoVaultAddressWhenNeitherPerCallNorConfigIsSet)
{
    ApiConfig config;
    config.env = Environment::Mainnet;
    config.wallet = Wallet{"", TEST_PRIVATE_KEY};

    auto result = Signing::prepareBodyForType(config, RestEndpointType::PlaceOrder, actionBody());

    EXPECT_FALSE(result.contains("vaultAddress"));
}

TEST(SigningPrepareBodyForType, VaultTransferNeverPicksUpConfigVaultAddressFallback)
{
    // vaultTransfer's target vault is a field of the action itself, not the wrapper - so unlike
    // PlaceOrder above, ApiConfig::vaultAddress must never leak into its request body.
    ApiConfig config;
    config.env = Environment::Mainnet;
    config.wallet = Wallet{"", TEST_PRIVATE_KEY};
    config.vaultAddress = "0x2222222222222222222222222222222222222222";

    auto result = Signing::prepareBodyForType(config, RestEndpointType::VaultTransfer, actionBody());

    EXPECT_FALSE(result.contains("vaultAddress"));
}

TEST(SigningPrepareBodyForType, Hip3LiquidatorTransferNeverPicksUpConfigVaultAddressFallback)
{
    ApiConfig config;
    config.env = Environment::Mainnet;
    config.wallet = Wallet{"", TEST_PRIVATE_KEY};
    config.vaultAddress = "0x2222222222222222222222222222222222222222";

    auto result = Signing::prepareBodyForType(config, RestEndpointType::Hip3LiquidatorTransfer, actionBody());

    EXPECT_FALSE(result.contains("vaultAddress"));
}

TEST(SigningPrepareBodyForType, UserSignedActionIgnoresVaultAddressEntirely)
{
    // usdSend is an EIP-712 user-signed action (see isUserSignedAction) - it's routed to
    // prepareUserSignedActionBody, which never reads vaultAddress at all, regardless of whether
    // one is passed per-call or configured as a default.
    ApiConfig config;
    config.env = Environment::Mainnet;
    config.wallet = Wallet{"", TEST_PRIVATE_KEY};
    config.vaultAddress = "0x2222222222222222222222222222222222222222";

    nlohmann::ordered_json body;
    body["action"]["destination"] = "0x5e9ee1089755c3435139848e47e6635505d5a13a";
    body["action"]["amount"] = "1";

    auto result = Signing::prepareBodyForType(
        config, RestEndpointType::UsdSend, body,
        std::string("0x1719884eb866cb12b2287399b15f7db5e7d775ea"));

    EXPECT_FALSE(result.contains("vaultAddress"));
    ASSERT_TRUE(result.contains("action"));
    EXPECT_FALSE(result["action"].contains("vaultAddress"));
}
