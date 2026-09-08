#include <gtest/gtest.h>

#include "messages/InfoRequestBuilder.h"
#include "messages/ExchangeRequestBuilder.h"
#include "hyperliquid/rest/RestApiMessageParser.h"

#include <nlohmann/json.hpp>

using namespace hyperliquid;

// --- Request building ---

TEST(InfoRequestBuilderTest, VaultDetailsWithoutUser)
{
    auto body = InfoRequestBuilder::vaultDetails("0xvault");
    EXPECT_EQ(body["type"], "vaultDetails");
    EXPECT_EQ(body["vaultAddress"], "0xvault");
    EXPECT_FALSE(body.contains("user"));
}

TEST(InfoRequestBuilderTest, VaultDetailsWithUser)
{
    auto body = InfoRequestBuilder::vaultDetails("0xvault", std::string("0xuser"));
    EXPECT_EQ(body["vaultAddress"], "0xvault");
    EXPECT_EQ(body["user"], "0xuser");
}

TEST(InfoRequestBuilderTest, UserVaultEquities)
{
    auto body = InfoRequestBuilder::userVaultEquities("0xabc");
    EXPECT_EQ(body["type"], "userVaultEquities");
    EXPECT_EQ(body["user"], "0xabc");
}

TEST(InfoRequestBuilderTest, Portfolio)
{
    auto body = InfoRequestBuilder::portfolio("0xabc");
    EXPECT_EQ(body["type"], "portfolio");
    EXPECT_EQ(body["user"], "0xabc");
}

TEST(InfoRequestBuilderTest, Referral)
{
    auto body = InfoRequestBuilder::referral("0xabc");
    EXPECT_EQ(body["type"], "referral");
    EXPECT_EQ(body["user"], "0xabc");
}

TEST(InfoRequestBuilderTest, UserRole)
{
    auto body = InfoRequestBuilder::userRole("0xabc");
    EXPECT_EQ(body["type"], "userRole");
    EXPECT_EQ(body["user"], "0xabc");
}

// --- createVault / setReferrer (exchange, L1 actions) ---
// Field shapes (action = {type, name, description, initialUsd} for createVault, plain L1-action
// signing with initialUsd converted to raw 1e6 USDC units like vaultTransfer's "usd"; action =
// {type, code} for setReferrer) are cross-checked against the official TS SDK
// (@nktkas/hyperliquid, src/api/exchange/_methods/{createVault,setReferrer}.ts), not a live
// capture - please double-check before relying on this. createVault's action also carries its
// own "nonce" field equal to the envelope nonce (injected downstream by Signing::prepareBody,
// see the RestEndpointType::CreateVault special-case next to agentSendAsset's), so it isn't
// present in the builder's own output below.

TEST(CreateVaultBuilder, BodyShapeAndFieldOrder)
{
    ExchangeRequestBuilder builder;

    CreateVaultRequest request;
    request.name = "My Vault";
    request.description = "A vault that does things.";
    request.initialUsd = 100.0;

    auto body = builder.createVault(request);
    ASSERT_TRUE(body.contains("action"));
    const auto& action = body["action"];
    EXPECT_EQ(action["type"], "createVault");
    EXPECT_EQ(action["name"], "My Vault");
    EXPECT_EQ(action["description"], "A vault that does things.");
    EXPECT_EQ(action["initialUsd"].get<uint64_t>(), 100000000ULL);

    auto keys = std::vector<std::string>();
    for (auto it = action.begin(); it != action.end(); ++it) keys.push_back(it.key());
    EXPECT_EQ(keys, (std::vector<std::string>{"type", "name", "description", "initialUsd"}));
}

TEST(CreateVaultBuilder, InitialUsdScalesFractionalDollarsToRawUnits)
{
    ExchangeRequestBuilder builder;

    CreateVaultRequest request;
    request.name = "My Vault";
    request.description = "A vault that does things.";
    request.initialUsd = 100.5;

    auto body = builder.createVault(request);
    EXPECT_EQ(body["action"]["initialUsd"].get<uint64_t>(), 100500000ULL);
}

TEST(SetReferrerBuilder, BodyShapeAndFieldOrder)
{
    ExchangeRequestBuilder builder;

    SetReferrerRequest request;
    request.code = "ABC123";

    auto body = builder.setReferrer(request);
    ASSERT_TRUE(body.contains("action"));
    const auto& action = body["action"];
    EXPECT_EQ(action["type"], "setReferrer");
    EXPECT_EQ(action["code"], "ABC123");

    auto keys = std::vector<std::string>();
    for (auto it = action.begin(); it != action.end(); ++it) keys.push_back(it.key());
    EXPECT_EQ(keys, (std::vector<std::string>{"type", "code"}));
}

// --- Response parsing ---
// Fixtures below are real payloads captured against Hyperliquid testnet, not fabricated -
// several diverged from what the original implementation assumed (see referral tests).

TEST(RestApiMessageParserVaultsTest, ParseVaultDetails)
{
    std::string message = R"({
        "name": "Liquidator",
        "vaultAddress": "0x1719884eb866cb12b2287399b15f7db5e7d775ea",
        "leader": "0x295afc65e3557c75dacdb88d495c1f8ceb068173",
        "description": "Liquidates positions on all coins as soon as they become liquidatable.",
        "portfolio": [["day", {"accountValueHistory": [[1788514080005, "2001.704118"]], "pnlHistory": [[1788514080005, "0.0"]], "vlm": "0.0"}]],
        "apr": 0.0,
        "followerState": null,
        "leaderFraction": 0.050032450320323885,
        "leaderCommission": 0.0,
        "followers": [{"user": "0x04c962ac53b6ca309902a5a96784e6fd64e77704", "vaultEquity": "0.9177635649", "pnl": "0.0013295649", "allTimePnl": "-0.0086734351", "daysFollowing": 597, "vaultEntryTime": 1736976712771, "lockupUntil": 1737484285796}],
        "maxDistributable": 3.001289,
        "maxWithdrawable": 0.0,
        "isClosed": false,
        "relationship": {"type": "normal"},
        "allowDeposits": true,
        "alwaysCloseOnWithdraw": false
    })";

    RestApiMessageParser parser;
    auto response = parser.parseVaultDetails(message);

    EXPECT_EQ(response.name, "Liquidator");
    EXPECT_EQ(response.leader, "0x295afc65e3557c75dacdb88d495c1f8ceb068173");
    EXPECT_DOUBLE_EQ(response.leaderFraction, 0.050032450320323885);
    EXPECT_FALSE(response.followerStateRaw.has_value());
    EXPECT_FALSE(response.isClosed);
    EXPECT_EQ(response.relationship.type, "normal");
    EXPECT_TRUE(response.allowDeposits);
    EXPECT_FALSE(response.alwaysCloseOnWithdraw);
    ASSERT_EQ(response.portfolio.size(), 1u);
    EXPECT_DOUBLE_EQ(response.portfolio[0].accountValueHistory[0].second, 2001.704118);
    ASSERT_EQ(response.followers.size(), 1u);
    EXPECT_EQ(response.followers[0].daysFollowing, 597);
    EXPECT_DOUBLE_EQ(response.followers[0].vaultEquity, 0.9177635649);
}

TEST(RestApiMessageParserVaultsTest, ParseUserVaultEquitiesEmpty)
{
    RestApiMessageParser parser;
    auto response = parser.parseUserVaultEquities("[]");
    EXPECT_TRUE(response.equities.empty());
}

TEST(RestApiMessageParserVaultsTest, ParseUserVaultEquities)
{
    std::string message = R"([{"vaultAddress": "0x1719884eb866cb12b2287399b15f7db5e7d775ea", "equity": "12.5"}])";

    RestApiMessageParser parser;
    auto response = parser.parseUserVaultEquities(message);

    ASSERT_EQ(response.equities.size(), 1u);
    EXPECT_EQ(response.equities[0].vaultAddress, "0x1719884eb866cb12b2287399b15f7db5e7d775ea");
    EXPECT_DOUBLE_EQ(response.equities[0].equity, 12.5);
}

TEST(RestApiMessageParserVaultsTest, ParsePortfolio)
{
    std::string message = R"([
        ["day", {"accountValueHistory": [[1788514080005, "418.19"]], "pnlHistory": [[1788514080005, "0.0"]], "vlm": "418.19"}],
        ["allTime", {"accountValueHistory": [], "pnlHistory": [], "vlm": "5600.79"}]
    ])";

    RestApiMessageParser parser;
    auto response = parser.parsePortfolio(message);

    ASSERT_EQ(response.periods.size(), 2u);
    EXPECT_EQ(response.periods[0].period, PortfolioPeriodType::Day);
    EXPECT_DOUBLE_EQ(response.periods[0].vlm, 418.19);
    EXPECT_EQ(response.periods[1].period, PortfolioPeriodType::AllTime);
    EXPECT_DOUBLE_EQ(response.periods[1].vlm, 5600.79);
}

TEST(RestApiMessageParserVaultsTest, ParseUserRole)
{
    RestApiMessageParser parser;
    auto response = parser.parseUserRole(R"({"role":"user"})");
    EXPECT_EQ(response.role, UserRoleType::User);
    EXPECT_FALSE(response.agentUser.has_value());
}

// referrerState.data's shape is a discriminated union keyed by `stage` - this is the exact
// real payload that originally crashed parsing (tokenToState's actual shape is an array
// containing one [tokenIndex, state] tuple, not a flat 2-element array; and referrerState's
// "needToTrade" stage carries `required`, not `code`/`referralStates` as first assumed).
TEST(RestApiMessageParserVaultsTest, ParseReferralNeedToTradeStage)
{
    std::string message = R"({
        "referredBy": null,
        "cumVlm": "5200.79",
        "unclaimedRewards": "0.0",
        "claimedRewards": "0.0",
        "builderRewards": "0.0",
        "referrerState": {"stage": "needToTrade", "data": {"required": "4799.21"}},
        "rewardHistory": [],
        "tokenToState": [[0, {"cumVlm": "5200.79", "unclaimedRewards": "0.0", "claimedRewards": "0.0", "builderRewards": "0.0"}]]
    })";

    RestApiMessageParser parser;
    auto response = parser.parseReferral(message);

    EXPECT_FALSE(response.referredBy.has_value());
    EXPECT_DOUBLE_EQ(response.cumVlm, 5200.79);
    ASSERT_TRUE(response.referrerState.has_value());
    EXPECT_EQ(response.referrerState->stage, "needToTrade");
    ASSERT_TRUE(response.referrerState->required.has_value());
    EXPECT_DOUBLE_EQ(*response.referrerState->required, 4799.21);
    EXPECT_FALSE(response.referrerState->code.has_value());
    ASSERT_TRUE(response.tokenToState.has_value());
    EXPECT_EQ(response.tokenToState->first, 0);
    EXPECT_DOUBLE_EQ(response.tokenToState->second.cumVlm, 5200.79);
}

TEST(RestApiMessageParserVaultsTest, ParseReferralWithReferredBy)
{
    std::string message = R"({
        "referredBy": {"referrer": "0xreferrer", "code": "ABC123"},
        "cumVlm": "0.0",
        "unclaimedRewards": "0.0",
        "claimedRewards": "0.0",
        "builderRewards": "0.0",
        "referrerState": null,
        "rewardHistory": [],
        "tokenToState": null
    })";

    RestApiMessageParser parser;
    auto response = parser.parseReferral(message);

    ASSERT_TRUE(response.referredBy.has_value());
    EXPECT_EQ(response.referredBy->referrer, "0xreferrer");
    EXPECT_EQ(response.referredBy->code, "ABC123");
    EXPECT_FALSE(response.referrerState.has_value());
    EXPECT_FALSE(response.tokenToState.has_value());
}

// createVault's success response shape ({"status":"ok","response":{"type":"createVault",
// "data":"0x..."}}) is cross-checked against the official TS SDK's CreateVaultResponse type,
// not a live capture - please double-check before relying on this.
TEST(RestApiMessageParserVaultsTest, ParseCreateVaultSuccess)
{
    std::string message = R"({
        "status": "ok",
        "response": {
            "type": "createVault",
            "data": "0x1719884eb866cb12b2287399b15f7db5e7d775ea"
        }
    })";

    RestApiMessageParser parser;
    auto response = parser.parseCreateVault(message);

    EXPECT_EQ(response.status, "ok");
    EXPECT_EQ(response.type, "createVault");
    ASSERT_TRUE(response.vaultAddress.has_value());
    EXPECT_EQ(*response.vaultAddress, "0x1719884eb866cb12b2287399b15f7db5e7d775ea");
    EXPECT_FALSE(response.error.has_value());
}

TEST(RestApiMessageParserVaultsTest, ParseCreateVaultError)
{
    std::string message = R"({
        "status": "err",
        "response": "Initial balance must be at least 100 USD."
    })";

    RestApiMessageParser parser;
    auto response = parser.parseCreateVault(message);

    EXPECT_EQ(response.status, "err");
    ASSERT_TRUE(response.error.has_value());
    EXPECT_EQ(*response.error, "Initial balance must be at least 100 USD.");
    EXPECT_FALSE(response.vaultAddress.has_value());
}

// setReferrer returns SimpleResponse's usual {"status":"ok","response":{"type":"default"}}
// shape, same as vaultTransfer/hip3LiquidatorTransfer above.
TEST(RestApiMessageParserVaultsTest, ParseSetReferrerSuccess)
{
    std::string message = R"({
        "status": "ok",
        "response": {
            "type": "default"
        }
    })";

    RestApiMessageParser parser;
    auto response = parser.parseSimpleResponse(message);

    EXPECT_EQ(response.status, "ok");
    EXPECT_FALSE(response.error.has_value());
}

TEST(RestApiMessageParserVaultsTest, ParseSetReferrerError)
{
    std::string message = R"({
        "status": "err",
        "response": "Referral code does not exist."
    })";

    RestApiMessageParser parser;
    auto response = parser.parseSimpleResponse(message);

    EXPECT_EQ(response.status, "err");
    ASSERT_TRUE(response.error.has_value());
    EXPECT_EQ(*response.error, "Referral code does not exist.");
}
