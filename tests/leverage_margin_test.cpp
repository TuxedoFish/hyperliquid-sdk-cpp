#include <gtest/gtest.h>

#include <stdexcept>

#include "hyperliquid/config/Config.h"
#include "messages/ExchangeRequestBuilder.h"
#include "signing/Signing.h"

using namespace hyperliquid;

static const std::string kDummyPrivateKey =
    "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";

TEST(ExchangeRequestBuilderTest, UpdateLeverageCrossBodyShape)
{
    ExchangeRequestBuilder builder;

    UpdateLeverageRequest req;
    req.asset = "ETH";
    req.assetId = 4; // bypass symbol map resolution (no network in a unit test)
    req.isCross = true;
    req.leverage = 10;

    auto body = builder.updateLeverage(req);
    ASSERT_TRUE(body.contains("action"));

    auto action = body["action"];
    EXPECT_EQ(action["type"], "updateLeverage");
    EXPECT_EQ(action["asset"], 4);
    EXPECT_EQ(action["isCross"], true);
    EXPECT_EQ(action["leverage"], 10);
}

TEST(ExchangeRequestBuilderTest, UpdateLeverageIsolatedBodyShape)
{
    ExchangeRequestBuilder builder;

    UpdateLeverageRequest req;
    req.asset = "BTC";
    req.assetId = 0;
    req.isCross = false;
    req.leverage = 3;

    auto action = builder.updateLeverage(req)["action"];
    EXPECT_EQ(action["type"], "updateLeverage");
    EXPECT_EQ(action["asset"], 0);
    EXPECT_EQ(action["isCross"], false);
    EXPECT_EQ(action["leverage"], 3);
}

TEST(ExchangeRequestBuilderTest, UpdateLeverageResolvesUnknownSymbolThrows)
{
    ExchangeRequestBuilder builder;

    UpdateLeverageRequest req;
    req.asset = "ETH";
    req.isCross = true;
    req.leverage = 5;
    // No assetId supplied -> forces coin-symbol resolution via the (empty, in this
    // unit test) symbol map, proving resolution is attempted rather than skipped.
    EXPECT_THROW(builder.updateLeverage(req), std::invalid_argument);
}

TEST(ExchangeRequestBuilderTest, UpdateIsolatedMarginAddBodyShape)
{
    ExchangeRequestBuilder builder;

    UpdateIsolatedMarginRequest req;
    req.asset = "ETH";
    req.assetId = 4;
    req.isBuy = true;
    req.ntli = 1000000; // 1,000,000 == $1 (6 decimals)

    auto body = builder.updateIsolatedMargin(req);
    ASSERT_TRUE(body.contains("action"));

    auto action = body["action"];
    EXPECT_EQ(action["type"], "updateIsolatedMargin");
    EXPECT_EQ(action["asset"], 4);
    EXPECT_EQ(action["isBuy"], true);
    EXPECT_EQ(action["ntli"], 1000000);
}

TEST(ExchangeRequestBuilderTest, UpdateIsolatedMarginRemoveBodyShape)
{
    ExchangeRequestBuilder builder;

    UpdateIsolatedMarginRequest req;
    req.asset = "BTC";
    req.assetId = 0;
    req.isBuy = false;
    req.ntli = -500000; // remove $0.50 of margin

    auto action = builder.updateIsolatedMargin(req)["action"];
    EXPECT_EQ(action["asset"], 0);
    EXPECT_EQ(action["isBuy"], false);
    EXPECT_EQ(action["ntli"], -500000);
}

TEST(ExchangeRequestBuilderTest, UpdateIsolatedMarginResolvesUnknownSymbolThrows)
{
    ExchangeRequestBuilder builder;

    UpdateIsolatedMarginRequest req;
    req.asset = "BTC";
    req.isBuy = true;
    req.ntli = 1000000;
    EXPECT_THROW(builder.updateIsolatedMargin(req), std::invalid_argument);
}

TEST(ExchangeRequestBuilderTest, UpdateLeverageActionIsSignable)
{
    ExchangeRequestBuilder builder;

    UpdateLeverageRequest req;
    req.asset = "ETH";
    req.assetId = 4;
    req.isCross = true;
    req.leverage = 10;

    auto action = builder.updateLeverage(req)["action"];

    Wallet wallet{"", kDummyPrivateKey};
    auto sig = Signing::signL1Action(wallet, action, std::nullopt, 0, std::nullopt, false);
    EXPECT_FALSE(sig.r.empty());
    EXPECT_FALSE(sig.s.empty());
}

TEST(ExchangeRequestBuilderTest, UpdateIsolatedMarginActionIsSignable)
{
    ExchangeRequestBuilder builder;

    UpdateIsolatedMarginRequest req;
    req.asset = "ETH";
    req.assetId = 4;
    req.isBuy = true;
    req.ntli = 1000000;

    auto action = builder.updateIsolatedMargin(req)["action"];

    Wallet wallet{"", kDummyPrivateKey};
    auto sig = Signing::signL1Action(wallet, action, std::nullopt, 0, std::nullopt, false);
    EXPECT_FALSE(sig.r.empty());
    EXPECT_FALSE(sig.s.empty());
}
