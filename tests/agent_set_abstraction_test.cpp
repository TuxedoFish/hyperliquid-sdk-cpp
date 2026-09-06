#include <gtest/gtest.h>

#include "messages/ExchangeRequestBuilder.h"
#include "hyperliquid/rest/RestApiMessageParser.h"

using namespace hyperliquid;

// Request shape verified against nktkas/hyperliquid's AgentSetAbstractionRequest schema:
// action = { type: "agentSetAbstraction", abstraction: "i" | "u" | "p" }, signed as an L1 action.

TEST(AgentSetAbstractionBuilder, DisabledBodyShape)
{
    ExchangeRequestBuilder builder;

    auto body = builder.agentSetAbstraction(UserAbstractionMode::Disabled);

    ASSERT_TRUE(body.contains("action"));
    const auto& action = body["action"];
    EXPECT_EQ(action["type"], "agentSetAbstraction");
    EXPECT_EQ(action["abstraction"], "i");
}

TEST(AgentSetAbstractionBuilder, UnifiedAccountBodyShape)
{
    ExchangeRequestBuilder builder;

    auto body = builder.agentSetAbstraction(UserAbstractionMode::UnifiedAccount);

    const auto& action = body["action"];
    EXPECT_EQ(action["type"], "agentSetAbstraction");
    EXPECT_EQ(action["abstraction"], "u");
}

TEST(AgentSetAbstractionBuilder, PortfolioMarginBodyShape)
{
    ExchangeRequestBuilder builder;

    auto body = builder.agentSetAbstraction(UserAbstractionMode::PortfolioMargin);

    const auto& action = body["action"];
    EXPECT_EQ(action["type"], "agentSetAbstraction");
    EXPECT_EQ(action["abstraction"], "p");
}

TEST(UserAbstractionModeConversion, RoundTrips)
{
    EXPECT_EQ(toString(UserAbstractionMode::Disabled), "i");
    EXPECT_EQ(toString(UserAbstractionMode::UnifiedAccount), "u");
    EXPECT_EQ(toString(UserAbstractionMode::PortfolioMargin), "p");

    EXPECT_EQ(stringToUserAbstractionMode("i"), UserAbstractionMode::Disabled);
    EXPECT_EQ(stringToUserAbstractionMode("u"), UserAbstractionMode::UnifiedAccount);
    EXPECT_EQ(stringToUserAbstractionMode("p"), UserAbstractionMode::PortfolioMargin);

    EXPECT_THROW(stringToUserAbstractionMode("x"), std::invalid_argument);
}

// The error fixture below is real, captured live - including with a freshly-generated,
// properly-approved agent wallet (via approveAgent), ruling out signer identity as the cause.
// The equivalent userSetAbstraction call succeeds on the same account moments apart, so this is
// a real exchange-side restriction on this specific action, not an SDK request-shape issue - see
// examples/rest_agent_set_abstraction.cpp, which reproduces this end-to-end. The success fixture
// stays synthetic-but-schema-accurate, matching the official TS SDK's documented response shape,
// since a genuine "ok" wasn't obtainable during testing.

TEST(AgentSetAbstractionResponseParsing, SuccessResponse)
{
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

TEST(AgentSetAbstractionResponseParsing, ErrorResponse)
{
    // Real testnet response: signed with the account's own key rather than a distinct approved
    // agent wallet - rejected regardless of target mode (both UnifiedAccount and Disabled were
    // tried live and got this same error).
    static const std::string kErr = R"({
        "status": "err",
        "response": "Abstraction transition not allowed"
    })";

    RestApiMessageParser parser;
    auto resp = parser.parseSimpleResponse(kErr);

    EXPECT_EQ(resp.status, "err");
    ASSERT_TRUE(resp.error.has_value());
    EXPECT_EQ(*resp.error, "Abstraction transition not allowed");
}
