#include <gtest/gtest.h>

#include "messages/InfoRequestBuilder.h"
#include "hyperliquid/rest/RestApiMessageParser.h"

#include <nlohmann/json.hpp>

using namespace hyperliquid;

TEST(InfoRequestBuilderTest, MetaAndAssetCtxsNoDex)
{
    auto body = InfoRequestBuilder::metaAndAssetCtxs();
    EXPECT_EQ(body["type"], "metaAndAssetCtxs");
    EXPECT_FALSE(body.contains("dex"));
}

TEST(InfoRequestBuilderTest, MetaAndAssetCtxsWithDex)
{
    auto body = InfoRequestBuilder::metaAndAssetCtxs("test-dex");
    EXPECT_EQ(body["dex"], "test-dex");
}

TEST(InfoRequestBuilderTest, SpotMetaAndAssetCtxs)
{
    auto body = InfoRequestBuilder::spotMetaAndAssetCtxs();
    EXPECT_EQ(body["type"], "spotMetaAndAssetCtxs");
}

TEST(InfoRequestBuilderTest, SpotClearinghouseState)
{
    auto body = InfoRequestBuilder::spotClearinghouseState("0xabc");
    EXPECT_EQ(body["type"], "spotClearinghouseState");
    EXPECT_EQ(body["user"], "0xabc");
}

TEST(RestApiMessageParserInfoTest, ParseSpotMeta)
{
    // Real testnet response, trimmed to 3 tokens covering the evmContract-present/fullName-null,
    // evmContract-present/fullName-present, and evmContract-null/non-canonical variants. The
    // parser only reads "tokens" - the real response's "universe" (trading-pair) array isn't
    // currently parsed by parseSpotMeta.
    std::string message = R"({"tokens": [
        {"name": "USDC", "szDecimals": 8, "weiDecimals": 8, "index": 0, "tokenId": "0xeb62eee3685fc4c43992febcd9e75443", "isCanonical": true, "evmContract": {"address": "0x0b80659a4076e9e93c7dbe0f10675a16a3e5c206", "evm_extra_wei_decimals": -2}, "fullName": null},
        {"name": "PURR", "szDecimals": 0, "weiDecimals": 5, "index": 1, "tokenId": "0xc4bf3f870c0e9465323c0b6ed28096c2", "isCanonical": true, "evmContract": {"address": "0xa9056c15938f9aff34cd497c722ce33db0c2fd57", "evm_extra_wei_decimals": 13}, "fullName": "Hypurr"},
        {"name": "TEST", "szDecimals": 1, "weiDecimals": 8, "index": 2, "tokenId": "0x98b101daf4ff26697646131261c100bf", "isCanonical": false, "evmContract": null, "fullName": null}
    ]})";

    RestApiMessageParser parser;
    auto response = parser.parseSpotMeta(message);

    ASSERT_EQ(response.tokens.size(), 3u);
    EXPECT_EQ(response.tokens[0].name, "USDC");
    ASSERT_TRUE(response.tokens[0].evmContract.has_value());
    EXPECT_EQ(response.tokens[0].evmContract->address, "0x0b80659a4076e9e93c7dbe0f10675a16a3e5c206");
    EXPECT_FALSE(response.tokens[0].fullName.has_value());
    EXPECT_EQ(response.tokens[1].name, "PURR");
    ASSERT_TRUE(response.tokens[1].fullName.has_value());
    EXPECT_EQ(*response.tokens[1].fullName, "Hypurr");
    EXPECT_EQ(response.tokens[2].name, "TEST");
    EXPECT_FALSE(response.tokens[2].isCanonical);
    EXPECT_FALSE(response.tokens[2].evmContract.has_value());
}

TEST(RestApiMessageParserInfoTest, ParseOutcomeMeta)
{
    // Real testnet response, trimmed to 2 outcomes - one with an empty description, one with
    // the pipe-delimited "class:X|underlying:Y|..." format parsed by parseOutcomeDescription.
    std::string message = R"({"outcomes": [
        {"outcome": 10217, "name": "Fallback", "description": "", "sideSpecs": [{"name": "Yes"}, {"name": "No"}], "quoteToken": "USDC"},
        {"outcome": 17495, "name": "Recurring", "description": "class:priceBinary|underlying:BTC|expiry:20260907-0300|targetPrice:80115|period:1d", "sideSpecs": [{"name": "Yes"}, {"name": "No"}], "quoteToken": "USDC"}
    ]})";

    RestApiMessageParser parser;
    auto response = parser.parseOutcomeMeta(message);

    ASSERT_EQ(response.outcomes.size(), 2u);
    EXPECT_EQ(response.outcomes[0].outcome, 10217);
    EXPECT_EQ(response.outcomes[0].name, "Fallback");
    EXPECT_TRUE(response.outcomes[0].descriptionRaw.empty());
    EXPECT_EQ(response.outcomes[1].outcome, 17495);
    EXPECT_EQ(response.outcomes[1].description.outcomeClass, "priceBinary");
    EXPECT_EQ(response.outcomes[1].description.underlying, "BTC");
    EXPECT_EQ(response.outcomes[1].description.targetPrice, "80115");
}

TEST(RestApiMessageParserInfoTest, ParsePerpDexs)
{
    // Real testnet response, trimmed to the leading null (always the main dex - skipped by the
    // parser) plus 2 real HIP-3 dexes covering both the empty and populated
    // assetToStreamingOiCap/assetToFundingMultiplier variants.
    std::string message =
        R"([null,{"name":"test","fullName":"test dex","deployer":"0x5e89b26d8d66da9888c835c9bfcc2aa51813e152","oracleUpdater":null,"feeRecipient":null,"assetToStreamingOiCap":[],"assetToFundingMultiplier":[]},{"name":"felix","fullName":"felix","deployer":"0x3a4ca3a93fc224c0a073d087c19ba8f0f04c7f00","oracleUpdater":null,"feeRecipient":"0x3a4ca3a93fc224c0a073d087c19ba8f0f04c7f00","assetToStreamingOiCap":[["felix:CRCL","2500000.0"]],"assetToFundingMultiplier":[["felix:CRCL","1.0"]]}])";

    RestApiMessageParser parser;
    auto response = parser.parsePerpDexs(message);

    ASSERT_EQ(response.dexes.size(), 2u);
    EXPECT_EQ(response.dexes[0].name, "test");
    EXPECT_FALSE(response.dexes[0].feeRecipient.has_value());
    EXPECT_TRUE(response.dexes[0].assetToStreamingOiCap.empty());
    EXPECT_EQ(response.dexes[1].name, "felix");
    ASSERT_TRUE(response.dexes[1].feeRecipient.has_value());
    EXPECT_EQ(*response.dexes[1].feeRecipient, "0x3a4ca3a93fc224c0a073d087c19ba8f0f04c7f00");
    ASSERT_EQ(response.dexes[1].assetToStreamingOiCap.size(), 1u);
    EXPECT_EQ(response.dexes[1].assetToStreamingOiCap[0].first, "felix:CRCL");
}

TEST(RestApiMessageParserInfoTest, ParseMetaAndAssetCtxs)
{
    std::string message = R"([
        {"universe": [{"name": "BTC", "szDecimals": 5, "maxLeverage": 50}]},
        [{"dayNtlVlm": "1000.5", "prevDayPx": "60000.0", "markPx": "61000.0",
          "midPx": "61000.5", "funding": "0.0001", "openInterest": "500.0", "oraclePx": "61000.2"}]
    ])";

    RestApiMessageParser parser;
    auto response = parser.parseMetaAndAssetCtxs(message);

    ASSERT_EQ(response.meta.universe.size(), 1u);
    EXPECT_EQ(response.meta.universe[0].name, "BTC");
    ASSERT_EQ(response.assetCtxs.size(), 1u);
    EXPECT_EQ(response.assetCtxs[0].coin, "BTC");
    EXPECT_DOUBLE_EQ(response.assetCtxs[0].markPx, 61000.0);
    EXPECT_TRUE(response.assetCtxs[0].hasMidPx);
    EXPECT_DOUBLE_EQ(response.assetCtxs[0].midPx, 61000.5);
}

TEST(RestApiMessageParserInfoTest, ParseSpotMetaAndAssetCtxs)
{
    std::string message = R"([
        {
            "tokens": [{"name": "USDC", "szDecimals": 8, "weiDecimals": 8, "index": 0, "tokenId": "0x1", "isCanonical": true}],
            "universe": [{"name": "PURR/USDC", "tokens": [1, 0], "index": 0, "isCanonical": true}]
        },
        [{"dayNtlVlm": "500.0", "prevDayPx": "0.5", "markPx": "0.51", "circulatingSupply": "1000000.0"}]
    ])";

    RestApiMessageParser parser;
    auto response = parser.parseSpotMetaAndAssetCtxs(message);

    ASSERT_EQ(response.meta.tokens.size(), 1u);
    EXPECT_EQ(response.meta.tokens[0].name, "USDC");
    EXPECT_TRUE(response.meta.tokens[0].isCanonical);
    ASSERT_EQ(response.meta.universe.size(), 1u);
    EXPECT_EQ(response.meta.universe[0].name, "PURR/USDC");
    EXPECT_TRUE(response.meta.universe[0].isCanonical);
    ASSERT_EQ(response.assetCtxs.size(), 1u);
    EXPECT_EQ(response.assetCtxs[0].coin, "PURR/USDC");
    EXPECT_DOUBLE_EQ(response.assetCtxs[0].circulatingSupply, 1000000.0);
}

TEST(RestApiMessageParserInfoTest, ParseSpotClearinghouseState)
{
    std::string message = R"({
        "balances": [
            {"coin": "USDC", "token": 0, "hold": "0.0", "total": "508.05193", "entryNtl": "500.0"}
        ]
    })";

    RestApiMessageParser parser;
    auto response = parser.parseSpotClearinghouseState(message);

    ASSERT_EQ(response.balances.size(), 1u);
    EXPECT_EQ(response.balances[0].coin, "USDC");
    EXPECT_DOUBLE_EQ(response.balances[0].total, 508.05193);
}
