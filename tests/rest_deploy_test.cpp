#include <gtest/gtest.h>

#include "messages/ExchangeRequestBuilder.h"
#include "messages/InfoRequestBuilder.h"
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

// spotDeploy is a large multi-variant action (10 sub-actions sharing `type: "spotDeploy"`); only
// the 5 variants making up the "create and launch a new spot token" flow are implemented here -
// see issue #91. All 5 share `type: "spotDeploy"` with exactly one nested key present as the
// discriminator, rather than a separate `type`/enum field. Field shapes confirmed against the
// official TS SDK (@nktkas/hyperliquid, src/api/exchange/_methods/spotDeploy.ts), not a live
// capture - a full live "happy path" deployment creates permanent testnet state and was out of
// scope for this pass.

TEST(SpotDeployBuilderTest, RegisterToken2)
{
    ExchangeRequestBuilder builder;

    SpotDeployRegisterToken2Request request;
    request.spec.name = "TEST5";
    request.spec.szDecimals = 1;
    request.spec.weiDecimals = 7;
    request.maxGas = 5000;
    request.fullName = "Test Token Five";

    auto body = builder.spotDeployRegisterToken2(request);
    const auto& action = body["action"];
    EXPECT_EQ(action["type"], "spotDeploy");
    ASSERT_TRUE(action.contains("registerToken2"));
    const auto& registerToken2 = action["registerToken2"];
    EXPECT_EQ(registerToken2["spec"]["name"], "TEST5");
    EXPECT_EQ(registerToken2["spec"]["szDecimals"].get<int>(), 1);
    EXPECT_EQ(registerToken2["spec"]["weiDecimals"].get<int>(), 7);
    EXPECT_EQ(registerToken2["maxGas"].get<uint64_t>(), 5000ULL);
    EXPECT_EQ(registerToken2["fullName"], "Test Token Five");
}

TEST(SpotDeployBuilderTest, RegisterToken2OmitsFullNameWhenAbsent)
{
    ExchangeRequestBuilder builder;

    SpotDeployRegisterToken2Request request;
    request.spec.name = "TEST5";
    request.spec.szDecimals = 1;
    request.spec.weiDecimals = 7;
    request.maxGas = 5000;

    auto body = builder.spotDeployRegisterToken2(request);
    EXPECT_FALSE(body["action"]["registerToken2"].contains("fullName"));
}

TEST(SpotDeployBuilderTest, UserGenesis)
{
    ExchangeRequestBuilder builder;

    SpotDeployUserGenesisRequest request;
    request.token = 40;
    request.userAndWei = {{"0x051dbfc562d44e4a01ebb986da35a47ab4f346db", 100.5}};
    request.existingTokenAndWei = {{1, 50.25}};
    request.blacklistUsers = std::vector<std::pair<std::string, bool>>{
        {"0xf43d54c219b52269831784969b9e7c54f0d448c8", true}};

    auto body = builder.spotDeployUserGenesis(request);
    const auto& action = body["action"];
    EXPECT_EQ(action["type"], "spotDeploy");
    const auto& userGenesis = action["userGenesis"];
    EXPECT_EQ(userGenesis["token"].get<int>(), 40);
    ASSERT_EQ(userGenesis["userAndWei"].size(), 1u);
    EXPECT_EQ(userGenesis["userAndWei"][0][0], "0x051dbfc562d44e4a01ebb986da35a47ab4f346db");
    EXPECT_EQ(userGenesis["userAndWei"][0][1], "100.5");
    ASSERT_EQ(userGenesis["existingTokenAndWei"].size(), 1u);
    EXPECT_EQ(userGenesis["existingTokenAndWei"][0][0].get<int>(), 1);
    EXPECT_EQ(userGenesis["existingTokenAndWei"][0][1], "50.25");
    ASSERT_EQ(userGenesis["blacklistUsers"].size(), 1u);
    EXPECT_EQ(userGenesis["blacklistUsers"][0][0], "0xf43d54c219b52269831784969b9e7c54f0d448c8");
    EXPECT_EQ(userGenesis["blacklistUsers"][0][1].get<bool>(), true);
}

TEST(SpotDeployBuilderTest, UserGenesisOmitsBlacklistUsersWhenAbsent)
{
    ExchangeRequestBuilder builder;

    SpotDeployUserGenesisRequest request;
    request.token = 40;
    request.userAndWei = {};
    request.existingTokenAndWei = {};

    auto body = builder.spotDeployUserGenesis(request);
    EXPECT_FALSE(body["action"]["userGenesis"].contains("blacklistUsers"));
}

TEST(SpotDeployBuilderTest, Genesis)
{
    ExchangeRequestBuilder builder;

    SpotDeployGenesisRequest request;
    request.token = 40;
    request.maxSupply = 10000.0012234;
    request.noHyperliquidity = true;

    auto body = builder.spotDeployGenesis(request);
    const auto& action = body["action"];
    EXPECT_EQ(action["type"], "spotDeploy");
    const auto& genesis = action["genesis"];
    EXPECT_EQ(genesis["token"].get<int>(), 40);
    EXPECT_EQ(genesis["maxSupply"], "10000.0012234");
    EXPECT_EQ(genesis["noHyperliquidity"].get<bool>(), true);
}

TEST(SpotDeployBuilderTest, GenesisOmitsNoHyperliquidityWhenAbsent)
{
    ExchangeRequestBuilder builder;

    SpotDeployGenesisRequest request;
    request.token = 40;
    request.maxSupply = 10000.0;

    auto body = builder.spotDeployGenesis(request);
    EXPECT_FALSE(body["action"]["genesis"].contains("noHyperliquidity"));
}

TEST(SpotDeployBuilderTest, RegisterSpot)
{
    ExchangeRequestBuilder builder;

    SpotDeployRegisterSpotRequest request;
    request.baseToken = 40;
    request.quoteToken = 0;

    auto body = builder.spotDeployRegisterSpot(request);
    const auto& action = body["action"];
    EXPECT_EQ(action["type"], "spotDeploy");
    const auto& tokens = action["registerSpot"]["tokens"];
    ASSERT_EQ(tokens.size(), 2u);
    EXPECT_EQ(tokens[0].get<int>(), 40);
    EXPECT_EQ(tokens[1].get<int>(), 0);
}

TEST(SpotDeployBuilderTest, RegisterHyperliquidity)
{
    ExchangeRequestBuilder builder;

    SpotDeployRegisterHyperliquidityRequest request;
    request.spot = 934;
    request.startPx = 1.5;
    request.orderSz = 100.0;
    request.nOrders = 10;
    request.nSeededLevels = 5;

    auto body = builder.spotDeployRegisterHyperliquidity(request);
    const auto& action = body["action"];
    EXPECT_EQ(action["type"], "spotDeploy");
    const auto& registerHyperliquidity = action["registerHyperliquidity"];
    EXPECT_EQ(registerHyperliquidity["spot"].get<int>(), 934);
    EXPECT_EQ(registerHyperliquidity["startPx"], "1.5");
    EXPECT_EQ(registerHyperliquidity["orderSz"], "100");
    EXPECT_EQ(registerHyperliquidity["nOrders"].get<int>(), 10);
    EXPECT_EQ(registerHyperliquidity["nSeededLevels"].get<int>(), 5);
}

TEST(SpotDeployBuilderTest, RegisterHyperliquidityOmitsNSeededLevelsWhenAbsent)
{
    ExchangeRequestBuilder builder;

    SpotDeployRegisterHyperliquidityRequest request;
    request.spot = 934;
    request.startPx = 1.5;
    request.orderSz = 100.0;
    request.nOrders = 10;

    auto body = builder.spotDeployRegisterHyperliquidity(request);
    EXPECT_FALSE(body["action"]["registerHyperliquidity"].contains("nSeededLevels"));
}
