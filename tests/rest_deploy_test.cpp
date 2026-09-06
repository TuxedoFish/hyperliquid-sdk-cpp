#include <gtest/gtest.h>

#include "messages/InfoRequestBuilder.h"
#include "messages/ExchangeRequestBuilder.h"
#include "hyperliquid/rest/RestApiMessageParser.h"

#include <nlohmann/json.hpp>

using namespace hyperliquid;

TEST(InfoRequestBuilderTest, PerpDexLimits)
{
    auto body = InfoRequestBuilder::perpDexLimits("hyna");
    EXPECT_EQ(body["type"], "perpDexLimits");
    EXPECT_EQ(body["dex"], "hyna");
}

TEST(InfoRequestBuilderTest, PerpDexStatus)
{
    auto body = InfoRequestBuilder::perpDexStatus("hyna");
    EXPECT_EQ(body["type"], "perpDexStatus");
    EXPECT_EQ(body["dex"], "hyna");
}

TEST(InfoRequestBuilderTest, PerpDeployAuctionStatus)
{
    // Unlike perpDexLimits/perpDexStatus, this endpoint takes no dex parameter - confirmed
    // against both the official TS SDK (@nktkas/hyperliquid) and real testnet responses,
    // which are identical whether or not a "dex" field is sent.
    auto body = InfoRequestBuilder::perpDeployAuctionStatus();
    EXPECT_EQ(body["type"], "perpDeployAuctionStatus");
    EXPECT_FALSE(body.contains("dex"));
}

TEST(InfoRequestBuilderTest, SpotDeployState)
{
    auto body = InfoRequestBuilder::spotDeployState("0x051dbfc562d44e4a01ebb986da35a47ab4f346db");
    EXPECT_EQ(body["type"], "spotDeployState");
    EXPECT_EQ(body["user"], "0x051dbfc562d44e4a01ebb986da35a47ab4f346db");
}

TEST(InfoRequestBuilderTest, SpotPairDeployAuctionStatus)
{
    auto body = InfoRequestBuilder::spotPairDeployAuctionStatus();
    EXPECT_EQ(body["type"], "spotPairDeployAuctionStatus");
    EXPECT_FALSE(body.contains("user"));
}

TEST(RestApiMessageParserInfoTest, ParsePerpDexLimitsNull)
{
    // Real testnet response for the main dex (empty string) or an unknown dex name.
    std::string message = "null";

    RestApiMessageParser parser;
    auto response = parser.parsePerpDexLimits(message);

    EXPECT_FALSE(response.exists);
}

TEST(RestApiMessageParserInfoTest, ParsePerpDexLimits)
{
    // Real testnet response for a live HIP-3 dex ("hyna").
    std::string message =
        R"({"totalOiCap":"50000000.0","oiSzCapPerPerp":"10000000000.0","maxTransferNtl":"1000000000.0","coinToOiCap":[["hyna:BTC","100000.0"]]})";

    RestApiMessageParser parser;
    auto response = parser.parsePerpDexLimits(message);

    EXPECT_TRUE(response.exists);
    EXPECT_DOUBLE_EQ(response.totalOiCap, 50000000.0);
    EXPECT_DOUBLE_EQ(response.oiSzCapPerPerp, 10000000000.0);
    EXPECT_DOUBLE_EQ(response.maxTransferNtl, 1000000000.0);
    ASSERT_EQ(response.coinToOiCap.size(), 1u);
    EXPECT_EQ(response.coinToOiCap[0].coin, "hyna:BTC");
    EXPECT_DOUBLE_EQ(response.coinToOiCap[0].oiCap, 100000.0);
}

TEST(RestApiMessageParserInfoTest, ParsePerpDexStatusNull)
{
    // Real testnet response for an unknown dex name.
    std::string message = "null";

    RestApiMessageParser parser;
    auto response = parser.parsePerpDexStatus(message);

    EXPECT_FALSE(response.exists);
}

TEST(RestApiMessageParserInfoTest, ParsePerpDexStatus)
{
    // Real testnet response for a live HIP-3 dex ("hyna").
    std::string message = R"({"totalNetDeposit":"2518.912535"})";

    RestApiMessageParser parser;
    auto response = parser.parsePerpDexStatus(message);

    EXPECT_TRUE(response.exists);
    EXPECT_DOUBLE_EQ(response.totalNetDeposit, 2518.912535);
}

TEST(RestApiMessageParserInfoTest, ParsePerpDeployAuctionStatusEndGasNull)
{
    // Real testnet response - the auction is still in progress, so endGas is null.
    std::string message = R"({"startTimeSeconds":1788613200,"durationSeconds":111600,"startGas":"500.0","currentGas":"500.0","endGas":null})";

    RestApiMessageParser parser;
    auto response = parser.parsePerpDeployAuctionStatus(message);

    EXPECT_EQ(response.startTimeSeconds, 1788613200ULL);
    EXPECT_EQ(response.durationSeconds, 111600ULL);
    EXPECT_DOUBLE_EQ(response.startGas, 500.0);
    ASSERT_TRUE(response.currentGas.has_value());
    EXPECT_DOUBLE_EQ(*response.currentGas, 500.0);
    EXPECT_FALSE(response.endGas.has_value());
}

TEST(RestApiMessageParserInfoTest, ParsePerpDeployAuctionStatusEndGasPresent)
{
    // Synthetic - covers the completed-auction case where endGas is a non-null string.
    std::string message = R"({"startTimeSeconds":1690000000,"durationSeconds":3600,"startGas":"1000000.0","currentGas":null,"endGas":"100000.0"})";

    RestApiMessageParser parser;
    auto response = parser.parsePerpDeployAuctionStatus(message);

    EXPECT_FALSE(response.currentGas.has_value());
    ASSERT_TRUE(response.endGas.has_value());
    EXPECT_DOUBLE_EQ(*response.endGas, 100000.0);
}

TEST(RestApiMessageParserInfoTest, ParseSpotPairDeployAuctionStatusCurrentGasSet)
{
    // Real testnet response - the auction is in progress, so currentGas is set and endGas is null.
    std::string message = R"({"startTimeSeconds":1788613200,"durationSeconds":111600,"startGas":"500.0","currentGas":"500.0","endGas":null})";

    RestApiMessageParser parser;
    auto response = parser.parseSpotPairDeployAuctionStatus(message);

    EXPECT_EQ(response.startTimeSeconds, 1788613200ULL);
    EXPECT_EQ(response.durationSeconds, 111600ULL);
    EXPECT_DOUBLE_EQ(response.startGas, 500.0);
    ASSERT_TRUE(response.currentGas.has_value());
    EXPECT_DOUBLE_EQ(*response.currentGas, 500.0);
    EXPECT_FALSE(response.endGas.has_value());
}

TEST(RestApiMessageParserInfoTest, ParseSpotPairDeployAuctionStatusEndGasPresent)
{
    // Synthetic - covers the completed-auction case where endGas is a non-null string.
    std::string message = R"({"startTimeSeconds":1690000000,"durationSeconds":3600,"startGas":"1000000.0","currentGas":null,"endGas":"100000.0"})";

    RestApiMessageParser parser;
    auto response = parser.parseSpotPairDeployAuctionStatus(message);

    EXPECT_FALSE(response.currentGas.has_value());
    ASSERT_TRUE(response.endGas.has_value());
    EXPECT_DOUBLE_EQ(*response.endGas, 100000.0);
}

TEST(RestApiMessageParserInfoTest, ParseSpotDeployStateEmpty)
{
    // Real testnet response for a user with no deployed spot tokens.
    std::string message =
        R"({"states":[],"gasAuction":{"startTimeSeconds":1788613200,"durationSeconds":111600,"startGas":"3971.80961464","currentGas":"2607.68693082","endGas":null}})";

    RestApiMessageParser parser;
    auto response = parser.parseSpotDeployState(message);

    EXPECT_TRUE(response.states.empty());
    EXPECT_EQ(response.gasAuction.startTimeSeconds, 1788613200ULL);
    ASSERT_TRUE(response.gasAuction.currentGas.has_value());
    EXPECT_DOUBLE_EQ(*response.gasAuction.currentGas, 2607.68693082);
}

TEST(RestApiMessageParserInfoTest, ParseSpotDeployStateWithFullNameAndMaxSupply)
{
    // Real testnet response for a deployer with a live token: non-null fullName/maxSupply,
    // and populated userGenesisBalances/existingTokenGenesisBalances.
    std::string message =
        R"({"states":[{"token":1000,"spec":{"name":"DVMLU","szDecimals":2,"weiDecimals":8},"fullName":"💀🏴‍☠️⚔️🌕🐦🦕💸","deployerTradingFeeShare":"0.0","spots":[934],"maxSupply":"100000000000.0","hyperliquidityGenesisBalance":"1000000000.0","totalGenesisBalanceWei":"10000000000000000000","userGenesisBalances":[["0x051dbfc562d44e4a01ebb986da35a47ab4f346db","0.0"],["0xf43d54c219b52269831784969b9e7c54f0d448c8","88888888888.8889007568"]],"existingTokenGenesisBalances":[[1,"555555555.5555554628"],[6,"555555555.5555554628"]],"blacklistUsers":[]}],"gasAuction":{"startTimeSeconds":1788613200,"durationSeconds":111600,"startGas":"3971.80961464","currentGas":"2607.92255346","endGas":null}})";

    RestApiMessageParser parser;
    auto response = parser.parseSpotDeployState(message);

    ASSERT_EQ(response.states.size(), 1u);
    const auto& state = response.states[0];
    EXPECT_EQ(state.token, 1000);
    EXPECT_EQ(state.spec.name, "DVMLU");
    EXPECT_EQ(state.spec.szDecimals, 2);
    EXPECT_EQ(state.spec.weiDecimals, 8);
    ASSERT_TRUE(state.fullName.has_value());
    EXPECT_EQ(*state.fullName, "💀🏴‍☠️⚔️🌕🐦🦕💸");
    EXPECT_DOUBLE_EQ(state.deployerTradingFeeShare, 0.0);
    ASSERT_EQ(state.spots.size(), 1u);
    EXPECT_EQ(state.spots[0], 934);
    ASSERT_TRUE(state.maxSupply.has_value());
    EXPECT_DOUBLE_EQ(*state.maxSupply, 100000000000.0);
    EXPECT_DOUBLE_EQ(state.hyperliquidityGenesisBalance, 1000000000.0);
    EXPECT_DOUBLE_EQ(state.totalGenesisBalanceWei, 10000000000000000000.0);
    ASSERT_EQ(state.userGenesisBalances.size(), 2u);
    EXPECT_EQ(state.userGenesisBalances[0].address, "0x051dbfc562d44e4a01ebb986da35a47ab4f346db");
    EXPECT_DOUBLE_EQ(state.userGenesisBalances[0].balance, 0.0);
    EXPECT_EQ(state.userGenesisBalances[1].address, "0xf43d54c219b52269831784969b9e7c54f0d448c8");
    EXPECT_DOUBLE_EQ(state.userGenesisBalances[1].balance, 88888888888.8889007568);
    ASSERT_EQ(state.existingTokenGenesisBalances.size(), 2u);
    EXPECT_EQ(state.existingTokenGenesisBalances[0].token, 1);
    EXPECT_DOUBLE_EQ(state.existingTokenGenesisBalances[0].balance, 555555555.5555554628);
    EXPECT_TRUE(state.blacklistUsers.empty());
    EXPECT_TRUE(response.gasAuction.currentGas.has_value());
    EXPECT_FALSE(response.gasAuction.endGas.has_value());
}

TEST(RestApiMessageParserInfoTest, ParseSpotDeployStateWithNullFullNameAndMaxSupply)
{
    // Real testnet response for a deployer whose token has no full name set, plus a second
    // token with a null maxSupply (uncapped) and no spot listing yet.
    std::string message =
        R"({"states":[{"token":40,"spec":{"name":"TEST5","szDecimals":1,"weiDecimals":7},"fullName":null,"deployerTradingFeeShare":"0.0","spots":[33],"maxSupply":"10000.0012234","hyperliquidityGenesisBalance":"0.0","totalGenesisBalanceWei":"0","userGenesisBalances":[],"existingTokenGenesisBalances":[],"blacklistUsers":[]},{"token":50,"spec":{"name":"ONE","szDecimals":0,"weiDecimals":5},"fullName":null,"deployerTradingFeeShare":"0.0","spots":[],"maxSupply":null,"hyperliquidityGenesisBalance":"0.0","totalGenesisBalanceWei":"0","userGenesisBalances":[],"existingTokenGenesisBalances":[],"blacklistUsers":[]}],"gasAuction":{"startTimeSeconds":1788613200,"durationSeconds":111600,"startGas":"3971.80961464","currentGas":"2607.76501543","endGas":null}})";

    RestApiMessageParser parser;
    auto response = parser.parseSpotDeployState(message);

    ASSERT_EQ(response.states.size(), 2u);
    EXPECT_FALSE(response.states[0].fullName.has_value());
    ASSERT_TRUE(response.states[0].maxSupply.has_value());
    EXPECT_DOUBLE_EQ(*response.states[0].maxSupply, 10000.0012234);
    ASSERT_EQ(response.states[0].spots.size(), 1u);
    EXPECT_EQ(response.states[0].spots[0], 33);

    EXPECT_FALSE(response.states[1].fullName.has_value());
    EXPECT_FALSE(response.states[1].maxSupply.has_value());
    EXPECT_TRUE(response.states[1].spots.empty());
}

// perpDeploy/registerAsset2 field shapes below (action = {type: "perpDeploy",
// registerAsset2: {maxGas, assetRequest: {coin, szDecimals, oraclePx, marginTableId,
// marginMode}, dex, schema}}, plain L1-action signing with no embedded nonce/hyperliquidChain
// fields) are verified against the official TS SDK (@nktkas/hyperliquid,
// src/api/exchange/_methods/perpDeploy.ts) and the HIP-3 deployer actions docs page, not a live
// capture of a successful deployment. Only registerAsset2 is implemented - the other 15
// perpDeploy sub-actions (setOracle, setFundingMultipliers, haltTrading, margin table config,
// fee config, sub-deployers, etc.) are out of scope for this issue.

TEST(PerpDeployRegisterAsset2Builder, NewDexIncludesSchemaAndMaxGas)
{
    ExchangeRequestBuilder builder;

    PerpDeployRegisterAsset2Request request;
    request.maxGas = 5000ULL;
    request.assetRequest.coin = "NEWCOIN";
    request.assetRequest.szDecimals = 2;
    request.assetRequest.oraclePx = 100.5;
    request.assetRequest.marginTableId = 3;
    request.assetRequest.marginMode = PerpMarginMode::NoCross;
    request.dex = "newdex";
    PerpDeploySchema schema;
    schema.fullName = "New Dex";
    schema.collateralToken = 0;
    schema.oracleUpdater = "0x051dbfc562d44e4a01ebb986da35a47ab4f346db";
    request.schema = schema;

    auto body = builder.perpDeployRegisterAsset2(request);

    ASSERT_TRUE(body.contains("action"));
    const auto& action = body["action"];
    EXPECT_EQ(action["type"], "perpDeploy");
    ASSERT_TRUE(action.contains("registerAsset2"));
    const auto& reg = action["registerAsset2"];

    EXPECT_EQ(reg["maxGas"].get<uint64_t>(), 5000ULL);

    const auto& assetRequest = reg["assetRequest"];
    EXPECT_EQ(assetRequest["coin"], "NEWCOIN");
    EXPECT_EQ(assetRequest["szDecimals"].get<uint32_t>(), 2u);
    EXPECT_EQ(assetRequest["oraclePx"], "100.5");
    EXPECT_EQ(assetRequest["marginTableId"].get<uint32_t>(), 3u);
    EXPECT_EQ(assetRequest["marginMode"], "noCross");

    EXPECT_EQ(reg["dex"], "newdex");

    ASSERT_TRUE(reg.contains("schema"));
    ASSERT_FALSE(reg["schema"].is_null());
    EXPECT_EQ(reg["schema"]["fullName"], "New Dex");
    EXPECT_EQ(reg["schema"]["collateralToken"].get<uint64_t>(), 0u);
    EXPECT_EQ(reg["schema"]["oracleUpdater"], "0x051dbfc562d44e4a01ebb986da35a47ab4f346db");
}

TEST(PerpDeployRegisterAsset2Builder, ExistingDexSendsNullSchema)
{
    ExchangeRequestBuilder builder;

    PerpDeployRegisterAsset2Request request;
    request.maxGas = std::nullopt;
    request.assetRequest.coin = "NEWCOIN2";
    request.assetRequest.szDecimals = 4;
    request.assetRequest.oraclePx = 0.001;
    request.assetRequest.marginTableId = 1;
    request.assetRequest.marginMode = PerpMarginMode::StrictIsolated;
    request.dex = "test";
    request.schema = std::nullopt;

    auto body = builder.perpDeployRegisterAsset2(request);

    const auto& reg = body["action"]["registerAsset2"];

    // maxGas/schema must be sent as explicit JSON null, not omitted, when not provided.
    ASSERT_TRUE(reg.contains("maxGas"));
    EXPECT_TRUE(reg["maxGas"].is_null());
    ASSERT_TRUE(reg.contains("schema"));
    EXPECT_TRUE(reg["schema"].is_null());

    EXPECT_EQ(reg["assetRequest"]["marginMode"], "strictIsolated");
    EXPECT_EQ(reg["dex"], "test");
}

TEST(PerpDeployRegisterAsset2Builder, SchemaWithNullOracleUpdater)
{
    ExchangeRequestBuilder builder;

    PerpDeployRegisterAsset2Request request;
    request.maxGas = std::nullopt;
    request.assetRequest.coin = "NEWCOIN3";
    request.assetRequest.szDecimals = 0;
    request.assetRequest.oraclePx = 1.0;
    request.assetRequest.marginTableId = 0;
    request.assetRequest.marginMode = PerpMarginMode::Normal;
    request.dex = "brandnewdex";
    PerpDeploySchema schema;
    schema.fullName = "Brand New Dex";
    schema.collateralToken = 5;
    schema.oracleUpdater = std::nullopt;
    request.schema = schema;

    auto body = builder.perpDeployRegisterAsset2(request);

    const auto& reg = body["action"]["registerAsset2"];
    ASSERT_FALSE(reg["schema"].is_null());
    EXPECT_EQ(reg["schema"]["collateralToken"].get<uint64_t>(), 5u);
    // null oracleUpdater means the deployer itself is assumed to be the oracle updater - must
    // still be sent explicitly, not omitted.
    ASSERT_TRUE(reg["schema"].contains("oracleUpdater"));
    EXPECT_TRUE(reg["schema"]["oracleUpdater"].is_null());
    EXPECT_EQ(reg["assetRequest"]["marginMode"], "normal");
}

TEST(PerpDeployRegisterAsset2ResponseParsing, SuccessResponse)
{
    // Synthetic - shape verified against the official TS SDK/docs; a genuine successful
    // deployment creates permanent, irreversible on-chain state, so this isn't live-captured.
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

TEST(PerpDeployRegisterAsset2ResponseParsing, ErrorResponse)
{
    // Synthetic - a plausible rejection message for a wallet that isn't the target dex's
    // deployer. This isn't a real, live-captured payload (deliberately not executed - see the
    // TODO in examples/rest_perp_deploy_action.cpp); parseSimpleResponse's shape is already
    // covered by real captures elsewhere (e.g. hip3_liquidator_transfer_test.cpp), so this only
    // exercises this test file's err-status wiring, not a claim about the exact wording.
    static const std::string kErr = R"({
        "status": "err",
        "response": "Only the dex deployer can register new assets."
    })";

    RestApiMessageParser parser;
    auto resp = parser.parseSimpleResponse(kErr);

    EXPECT_EQ(resp.status, "err");
    ASSERT_TRUE(resp.error.has_value());
}
