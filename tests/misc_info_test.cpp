#include <gtest/gtest.h>

#include "messages/InfoRequestBuilder.h"
#include "hyperliquid/rest/RestApiMessageParser.h"

using namespace hyperliquid;

// --- Request building ---

TEST(InfoRequestBuilderMiscTest, PerpsAtOpenInterestCapNoDex)
{
    auto body = InfoRequestBuilder::perpsAtOpenInterestCap();
    EXPECT_EQ(body["type"], "perpsAtOpenInterestCap");
    EXPECT_FALSE(body.contains("dex"));
}

TEST(InfoRequestBuilderMiscTest, PerpsAtOpenInterestCapWithDex)
{
    auto body = InfoRequestBuilder::perpsAtOpenInterestCap("test-dex");
    EXPECT_EQ(body["type"], "perpsAtOpenInterestCap");
    EXPECT_EQ(body["dex"], "test-dex");
}

TEST(InfoRequestBuilderMiscTest, PredictedFundings)
{
    auto body = InfoRequestBuilder::predictedFundings();
    EXPECT_EQ(body["type"], "predictedFundings");
}

TEST(InfoRequestBuilderMiscTest, PerpAnnotation)
{
    auto body = InfoRequestBuilder::perpAnnotation("BTC");
    EXPECT_EQ(body["type"], "perpAnnotation");
    EXPECT_EQ(body["coin"], "BTC");
}

TEST(InfoRequestBuilderMiscTest, PerpCategories)
{
    auto body = InfoRequestBuilder::perpCategories();
    EXPECT_EQ(body["type"], "perpCategories");
}

TEST(InfoRequestBuilderMiscTest, PerpConciseAnnotations)
{
    auto body = InfoRequestBuilder::perpConciseAnnotations();
    EXPECT_EQ(body["type"], "perpConciseAnnotations");
}

TEST(InfoRequestBuilderMiscTest, AllPerpMetas)
{
    auto body = InfoRequestBuilder::allPerpMetas();
    EXPECT_EQ(body["type"], "allPerpMetas");
}

// --- Response parsing ---

TEST(RestApiMessageParserMiscTest, ParsePerpsAtOpenInterestCap)
{
    std::string message = R"(["BADGER","CANTO","FTM","LOOM","PURR"])";

    RestApiMessageParser parser;
    auto response = parser.parsePerpsAtOpenInterestCap(message);

    ASSERT_EQ(response.coins.size(), 5u);
    EXPECT_EQ(response.coins[0], "BADGER");
    EXPECT_EQ(response.coins[4], "PURR");
}

TEST(RestApiMessageParserMiscTest, ParsePredictedFundings)
{
    std::string message = R"([
        [
            "AVAX",
            [
                ["BinPerp", {"fundingRate": "0.0001", "nextFundingTime": 1733961600000}],
                ["HlPerp", {"fundingRate": "0.0000125", "nextFundingTime": 1733958000000}],
                ["OkxPerp", {"fundingRate": null, "nextFundingTime": null}]
            ]
        ]
    ])";

    RestApiMessageParser parser;
    auto response = parser.parsePredictedFundings(message);

    ASSERT_EQ(response.fundings.size(), 1u);
    const auto& avax = response.fundings[0];
    EXPECT_EQ(avax.coin, "AVAX");
    ASSERT_EQ(avax.venues.size(), 3u);

    EXPECT_EQ(avax.venues[0].venue, "BinPerp");
    ASSERT_TRUE(avax.venues[0].fundingRate.has_value());
    EXPECT_DOUBLE_EQ(*avax.venues[0].fundingRate, 0.0001);
    ASSERT_TRUE(avax.venues[0].nextFundingTime.has_value());
    EXPECT_EQ(*avax.venues[0].nextFundingTime, 1733961600000ULL);

    EXPECT_EQ(avax.venues[1].venue, "HlPerp");
    ASSERT_TRUE(avax.venues[1].fundingRate.has_value());
    EXPECT_DOUBLE_EQ(*avax.venues[1].fundingRate, 0.0000125);

    EXPECT_EQ(avax.venues[2].venue, "OkxPerp");
    EXPECT_FALSE(avax.venues[2].fundingRate.has_value());
    EXPECT_FALSE(avax.venues[2].nextFundingTime.has_value());
}

TEST(RestApiMessageParserMiscTest, ParsePerpAnnotation)
{
    std::string message = R"({"category": "other", "description": "other perps"})";

    RestApiMessageParser parser;
    auto response = parser.parsePerpAnnotation(message);

    EXPECT_EQ(response.category, "other");
    EXPECT_EQ(response.description, "other perps");
}

TEST(RestApiMessageParserMiscTest, ParsePerpCategories)
{
    std::string message = R"([["birb:PENGU","test_cat"],["nq:TEST","preipo"],["nq:TEST1","all"],["nq:TEST2","ai"]])";

    RestApiMessageParser parser;
    auto response = parser.parsePerpCategories(message);

    ASSERT_EQ(response.categories.size(), 4u);
    EXPECT_EQ(response.categories[0].coin, "birb:PENGU");
    EXPECT_EQ(response.categories[0].category, "test_cat");
    EXPECT_EQ(response.categories[3].coin, "nq:TEST2");
    EXPECT_EQ(response.categories[3].category, "ai");
}

TEST(RestApiMessageParserMiscTest, ParsePerpConciseAnnotations)
{
    std::string message = R"([
        ["dex:CATS", {"category": "indices", "keywords": ["meow"]}],
        ["dex:DOGS", {"category": "indices"}]
    ])";

    RestApiMessageParser parser;
    auto response = parser.parsePerpConciseAnnotations(message);

    ASSERT_EQ(response.annotations.size(), 2u);
    EXPECT_EQ(response.annotations[0].coin, "dex:CATS");
    EXPECT_EQ(response.annotations[0].category, "indices");
    ASSERT_EQ(response.annotations[0].keywords.size(), 1u);
    EXPECT_EQ(response.annotations[0].keywords[0], "meow");

    EXPECT_EQ(response.annotations[1].coin, "dex:DOGS");
    EXPECT_EQ(response.annotations[1].category, "indices");
    EXPECT_TRUE(response.annotations[1].keywords.empty());
}

TEST(RestApiMessageParserMiscTest, ParseAllPerpMetas)
{
    // Each entry is a flat per-dex meta object - unlike metaAndAssetCtxs, this endpoint does
    // not pair each dex with live asset-context data (confirmed against real testnet responses;
    // an earlier version of this test assumed a [meta, assetCtxs] pair shape that doesn't match
    // what the exchange actually returns).
    std::string message = R"([
        {
            "universe": [
                {"name": "BTC", "szDecimals": 5, "maxLeverage": 50},
                {"name": "ETH", "szDecimals": 4, "maxLeverage": 50}
            ],
            "marginTables": [
                [50, {"description": "", "marginTiers": [{"lowerBound": "0.0", "maxLeverage": 50}]}],
                [51, {"description": "tiered 10x", "marginTiers": [
                    {"lowerBound": "0.0", "maxLeverage": 10},
                    {"lowerBound": "3000000.0", "maxLeverage": 5}
                ]}]
            ],
            "collateralToken": 0
        }
    ])";

    RestApiMessageParser parser;
    auto response = parser.parseAllPerpMetas(message);

    ASSERT_EQ(response.dexMetas.size(), 1u);
    const auto& dex = response.dexMetas[0];

    ASSERT_EQ(dex.universe.size(), 2u);
    EXPECT_EQ(dex.universe[0].name, "BTC");
    EXPECT_EQ(dex.universe[0].szDecimals, 5);
    EXPECT_EQ(dex.universe[0].maxLeverage, 50);
    EXPECT_EQ(dex.collateralToken, 0);

    ASSERT_EQ(dex.marginTables.size(), 2u);
    EXPECT_EQ(dex.marginTables[0].id, 50);
    ASSERT_EQ(dex.marginTables[0].marginTiers.size(), 1u);
    EXPECT_DOUBLE_EQ(dex.marginTables[0].marginTiers[0].lowerBound, 0.0);
    EXPECT_EQ(dex.marginTables[0].marginTiers[0].maxLeverage, 50);

    EXPECT_EQ(dex.marginTables[1].id, 51);
    EXPECT_EQ(dex.marginTables[1].description, "tiered 10x");
    ASSERT_EQ(dex.marginTables[1].marginTiers.size(), 2u);
    EXPECT_DOUBLE_EQ(dex.marginTables[1].marginTiers[1].lowerBound, 3000000.0);
    EXPECT_EQ(dex.marginTables[1].marginTiers[1].maxLeverage, 5);
}
