// Signing tests ported from hyperliquid-python-sdk
#include <gtest/gtest.h>

#include "signing/Signing.h"
#include "signing/SigningHelpers.h"

#include <nlohmann/json.hpp>

using namespace hyperliquid;

static const std::string TEST_PRIVATE_KEY = "0123456789012345678901234567890123456789012345678901234567890123";

static Wallet testWallet()
{
    return Wallet{"", TEST_PRIVATE_KEY};
}

static uint64_t floatToIntForHashing(double value)
{
    double withDecimals = value * 1e8;
    return static_cast<uint64_t>(std::round(withDecimals));
}

static nlohmann::ordered_json orderWireToAction(const nlohmann::ordered_json& orderWire,
                                                 const std::string& grouping = "na")
{
    nlohmann::ordered_json action;
    action["type"] = "order";
    action["orders"] = nlohmann::ordered_json::array({orderWire});
    action["grouping"] = grouping;
    return action;
}

// Mirrors Python's float_to_wire: Decimal(f"{x:.8f}").normalize()
static std::string floatToWire(double x)
{
    char buf[64];
    std::snprintf(buf, sizeof(buf), "%.8f", x);
    std::string s(buf);
    if (s.find('.') != std::string::npos)
    {
        size_t last = s.find_last_not_of('0');
        if (s[last] == '.')
            s = s.substr(0, last);
        else
            s = s.substr(0, last + 1);
    }
    return s;
}

static nlohmann::ordered_json makeLimitOrderWire(int asset, bool isBuy, double price, double size, const std::string& tif)
{
    nlohmann::ordered_json wire;
    wire["a"] = asset;
    wire["b"] = isBuy;
    wire["p"] = floatToWire(price);
    wire["s"] = floatToWire(size);
    wire["r"] = false;
    wire["t"] = {{"limit", {{"tif", tif}}}};
    return wire;
}

TEST(SigningTest, PhantomAgentCreationMatchesProduction)
{
    uint64_t timestamp = 1677777606040;

    auto orderWire = makeLimitOrderWire(4, true, 1670.1, 0.0147, "Ioc");
    auto action = orderWireToAction(orderWire);
    auto hash = SigningHelpers::actionHash(action, std::nullopt, timestamp, std::nullopt);

    std::string connectionIdHex = SigningHelpers::toHexPadded(hash.data(), 32);
    EXPECT_EQ(connectionIdHex, "0x0fcbeda5ae3c4950a548021552a4fea2226858c4453571bf3f24ba017eac2908");
}

TEST(SigningTest, L1ActionSigningMatches)
{
    auto wallet = testWallet();
    nlohmann::ordered_json action;
    action["type"] = "dummy";
    action["num"] = floatToIntForHashing(1000);

    auto sigMainnet = Signing::signL1Action(wallet, action, std::nullopt, 0, std::nullopt, true);
    EXPECT_EQ(sigMainnet.r, "0x53749d5b30552aeb2fca34b530185976545bb22d0b3ce6f62e31be961a59298");
    EXPECT_EQ(sigMainnet.s, "0x755c40ba9bf05223521753995abb2f73ab3229be8ec921f350cb447e384d8ed8");
    EXPECT_EQ(sigMainnet.v, 27);

    auto sigTestnet = Signing::signL1Action(wallet, action, std::nullopt, 0, std::nullopt, false);
    EXPECT_EQ(sigTestnet.r, "0x542af61ef1f429707e3c76c5293c80d01f74ef853e34b76efffcb57e574f9510");
    EXPECT_EQ(sigTestnet.s, "0x17b8b32f086e8cdede991f1e2c529f5dd5297cbe8128500e00cbaf766204a613");
    EXPECT_EQ(sigTestnet.v, 28);
}

TEST(SigningTest, L1ActionSigningOrderMatches)
{
    auto wallet = testWallet();
    auto orderWire = makeLimitOrderWire(1, true, 100, 100, "Gtc");
    auto action = orderWireToAction(orderWire);

    auto sigMainnet = Signing::signL1Action(wallet, action, std::nullopt, 0, std::nullopt, true);
    EXPECT_EQ(sigMainnet.r, "0xd65369825a9df5d80099e513cce430311d7d26ddf477f5b3a33d2806b100d78e");
    EXPECT_EQ(sigMainnet.s, "0x2b54116ff64054968aa237c20ca9ff68000f977c93289157748a3162b6ea940e");
    EXPECT_EQ(sigMainnet.v, 28);

    auto sigTestnet = Signing::signL1Action(wallet, action, std::nullopt, 0, std::nullopt, false);
    EXPECT_EQ(sigTestnet.r, "0x82b2ba28e76b3d761093aaded1b1cdad4960b3af30212b343fb2e6cdfa4e3d54");
    EXPECT_EQ(sigTestnet.s, "0x6b53878fc99d26047f4d7e8c90eb98955a109f44209163f52d8dc4278cbbd9f5");
    EXPECT_EQ(sigTestnet.v, 27);
}

TEST(SigningTest, L1ActionSigningOrderWithCloidMatches)
{
    auto wallet = testWallet();
    auto orderWire = makeLimitOrderWire(1, true, 100, 100, "Gtc");
    orderWire["c"] = "0x00000000000000000000000000000001";
    auto action = orderWireToAction(orderWire);

    auto sigMainnet = Signing::signL1Action(wallet, action, std::nullopt, 0, std::nullopt, true);
    EXPECT_EQ(sigMainnet.r, "0x41ae18e8239a56cacbc5dad94d45d0b747e5da11ad564077fcac71277a946e3");
    EXPECT_EQ(sigMainnet.s, "0x3c61f667e747404fe7eea8f90ab0e76cc12ce60270438b2058324681a00116da");
    EXPECT_EQ(sigMainnet.v, 27);

    auto sigTestnet = Signing::signL1Action(wallet, action, std::nullopt, 0, std::nullopt, false);
    EXPECT_EQ(sigTestnet.r, "0xeba0664bed2676fc4e5a743bf89e5c7501aa6d870bdb9446e122c9466c5cd16d");
    EXPECT_EQ(sigTestnet.s, "0x7f3e74825c9114bc59086f1eebea2928c190fdfbfde144827cb02b85bbe90988");
    EXPECT_EQ(sigTestnet.v, 28);
}

TEST(SigningTest, L1ActionSigningMatchesWithVault)
{
    auto wallet = testWallet();
    nlohmann::ordered_json action;
    action["type"] = "dummy";
    action["num"] = floatToIntForHashing(1000);

    std::string vaultAddress = "0x1719884eb866cb12b2287399b15f7db5e7d775ea";

    auto sigMainnet = Signing::signL1Action(wallet, action, vaultAddress, 0, std::nullopt, true);
    EXPECT_EQ(sigMainnet.r, "0x3c548db75e479f8012acf3000ca3a6b05606bc2ec0c29c50c515066a326239");
    EXPECT_EQ(sigMainnet.s, "0x4d402be7396ce74fbba3795769cda45aec00dc3125a984f2a9f23177b190da2c");
    EXPECT_EQ(sigMainnet.v, 28);

    auto sigTestnet = Signing::signL1Action(wallet, action, vaultAddress, 0, std::nullopt, false);
    EXPECT_EQ(sigTestnet.r, "0xe281d2fb5c6e25ca01601f878e4d69c965bb598b88fac58e475dd1f5e56c362b");
    EXPECT_EQ(sigTestnet.s, "0x7ddad27e9a238d045c035bc606349d075d5c5cd00a6cd1da23ab5c39d4ef0f60");
    EXPECT_EQ(sigTestnet.v, 27);
}

TEST(SigningTest, L1ActionSigningTpslOrderMatches)
{
    auto wallet = testWallet();

    nlohmann::ordered_json orderWire;
    orderWire["a"] = 1;
    orderWire["b"] = true;
    orderWire["p"] = floatToWire(100);
    orderWire["s"] = floatToWire(100);
    orderWire["r"] = false;
    orderWire["t"] = {{"trigger", {{"isMarket", true}, {"triggerPx", floatToWire(103)}, {"tpsl", "sl"}}}};

    auto action = orderWireToAction(orderWire);

    auto sigMainnet = Signing::signL1Action(wallet, action, std::nullopt, 0, std::nullopt, true);
    EXPECT_EQ(sigMainnet.r, "0x98343f2b5ae8e26bb2587daad3863bc70d8792b09af1841b6fdd530a2065a3f9");
    EXPECT_EQ(sigMainnet.s, "0x6b5bb6bb0633b710aa22b721dd9dee6d083646a5f8e581a20b545be6c1feb405");
    EXPECT_EQ(sigMainnet.v, 27);

    auto sigTestnet = Signing::signL1Action(wallet, action, std::nullopt, 0, std::nullopt, false);
    EXPECT_EQ(sigTestnet.r, "0x971c554d917c44e0e1b6cc45d8f9404f32172a9d3b3566262347d0302896a2e4");
    EXPECT_EQ(sigTestnet.s, "0x206257b104788f80450f8e786c329daa589aa0b32ba96948201ae556d5637eac");
    EXPECT_EQ(sigTestnet.v, 28);
}

TEST(SigningTest, FloatToIntForHashing)
{
    EXPECT_EQ(floatToIntForHashing(0.00001231), 1231ULL);
    EXPECT_EQ(floatToIntForHashing(1.033), 103300000ULL);
}

TEST(SigningTest, SignUsdTransferAction)
{
    auto wallet = testWallet();
    nlohmann::ordered_json action;
    action["type"] = "usdSend";
    action["hyperliquidChain"] = "Testnet";
    action["signatureChainId"] = "0x66eee";
    action["destination"] = "0x5e9ee1089755c3435139848e47e6635505d5a13a";
    action["amount"] = "1";
    action["time"] = 1687816341423;

    auto sig = Signing::signUserSignedAction(
        wallet, action,
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

TEST(SigningTest, SignWithdrawFromBridgeAction)
{
    auto wallet = testWallet();
    nlohmann::ordered_json action;
    action["type"] = "withdraw";
    action["hyperliquidChain"] = "Testnet";
    action["signatureChainId"] = "0x66eee";
    action["destination"] = "0x5e9ee1089755c3435139848e47e6635505d5a13a";
    action["amount"] = "1";
    action["time"] = 1687816341423;

    auto sig = Signing::signUserSignedAction(
        wallet, action,
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

TEST(SigningTest, CreateSubAccountAction)
{
    auto wallet = testWallet();
    nlohmann::ordered_json action;
    action["type"] = "createSubAccount";
    action["name"] = "example";

    auto sigMainnet = Signing::signL1Action(wallet, action, std::nullopt, 0, std::nullopt, true);
    EXPECT_EQ(sigMainnet.r, "0x51096fe3239421d16b671e192f574ae24ae14329099b6db28e479b86cdd6caa7");
    EXPECT_EQ(sigMainnet.s, "0xb71f7d293af92d3772572afb8b102d167a7cef7473388286bc01f52a5c5b423");
    EXPECT_EQ(sigMainnet.v, 27);

    auto sigTestnet = Signing::signL1Action(wallet, action, std::nullopt, 0, std::nullopt, false);
    EXPECT_EQ(sigTestnet.r, "0xa699e3ed5c2b89628c746d3298b5dc1cca604694c2c855da8bb8250ec8014a5b");
    EXPECT_EQ(sigTestnet.s, "0x53f1b8153a301c72ecc655b1c315d64e1dcea3ee58921fd7507e35818fcc1584");
    EXPECT_EQ(sigTestnet.v, 28);
}

TEST(SigningTest, SubAccountTransferAction)
{
    auto wallet = testWallet();
    nlohmann::ordered_json action;
    action["type"] = "subAccountTransfer";
    action["subAccountUser"] = "0x1d9470d4b963f552e6f671a81619d395877bf409";
    action["isDeposit"] = true;
    action["usd"] = 10;

    auto sigMainnet = Signing::signL1Action(wallet, action, std::nullopt, 0, std::nullopt, true);
    EXPECT_EQ(sigMainnet.r, "0x43592d7c6c7d816ece2e206f174be61249d651944932b13343f4d13f306ae602");
    EXPECT_EQ(sigMainnet.s, "0x71a926cb5c9a7c01c3359ec4c4c34c16ff8107d610994d4de0e6430e5cc0f4c9");
    EXPECT_EQ(sigMainnet.v, 28);

    auto sigTestnet = Signing::signL1Action(wallet, action, std::nullopt, 0, std::nullopt, false);
    EXPECT_EQ(sigTestnet.r, "0xe26574013395ad55ee2f4e0575310f003c5bb3351b5425482e2969fa51543927");
    EXPECT_EQ(sigTestnet.s, "0xefb08999196366871f919fd0e138b3a7f30ee33e678df7cfaf203e25f0a4278");
    EXPECT_EQ(sigTestnet.v, 28);
}

TEST(SigningTest, ScheduleCancelAction)
{
    auto wallet = testWallet();

    // Without time
    nlohmann::ordered_json action;
    action["type"] = "scheduleCancel";

    auto sigMainnet = Signing::signL1Action(wallet, action, std::nullopt, 0, std::nullopt, true);
    EXPECT_EQ(sigMainnet.r, "0x6cdfb286702f5917e76cd9b3b8bf678fcc49aec194c02a73e6d4f16891195df9");
    EXPECT_EQ(sigMainnet.s, "0x6557ac307fa05d25b8d61f21fb8a938e703b3d9bf575f6717ba21ec61261b2a0");
    EXPECT_EQ(sigMainnet.v, 27);

    auto sigTestnet = Signing::signL1Action(wallet, action, std::nullopt, 0, std::nullopt, false);
    EXPECT_EQ(sigTestnet.r, "0xc75bb195c3f6a4e06b7d395acc20bbb224f6d23ccff7c6a26d327304e6efaeed");
    EXPECT_EQ(sigTestnet.s, "0x342f8ede109a29f2c0723bd5efb9e9100e3bbb493f8fb5164ee3d385908233df");
    EXPECT_EQ(sigTestnet.v, 28);

    // With time
    nlohmann::ordered_json actionWithTime;
    actionWithTime["type"] = "scheduleCancel";
    actionWithTime["time"] = 123456789;

    sigMainnet = Signing::signL1Action(wallet, actionWithTime, std::nullopt, 0, std::nullopt, true);
    EXPECT_EQ(sigMainnet.r, "0x609cb20c737945d070716dcc696ba030e9976fcf5edad87afa7d877493109d55");
    EXPECT_EQ(sigMainnet.s, "0x16c685d63b5c7a04512d73f183b3d7a00da5406ff1f8aad33f8ae2163bab758b");
    EXPECT_EQ(sigMainnet.v, 28);

    sigTestnet = Signing::signL1Action(wallet, actionWithTime, std::nullopt, 0, std::nullopt, false);
    EXPECT_EQ(sigTestnet.r, "0x4e4f2dbd4107c69783e251b7e1057d9f2b9d11cee213441ccfa2be63516dc5bc");
    EXPECT_EQ(sigTestnet.s, "0x706c656b23428c8ba356d68db207e11139ede1670481a9e01ae2dfcdb0e1a678");
    EXPECT_EQ(sigTestnet.v, 27);
}
