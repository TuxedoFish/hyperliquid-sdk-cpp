#include <gtest/gtest.h>

#include "messages/InfoRequestBuilder.h"
#include "hyperliquid/rest/RestApiMessageParser.h"

#include <nlohmann/json.hpp>

using namespace hyperliquid;

// --- Request builder tests ---

TEST(InfoRequestBuilderTest, UserDexAbstractionState)
{
    auto body = InfoRequestBuilder::userDexAbstractionState("0xabc");
    EXPECT_EQ(body["type"], "userDexAbstraction");
    EXPECT_EQ(body["user"], "0xabc");
}

TEST(InfoRequestBuilderTest, UserAbstraction)
{
    auto body = InfoRequestBuilder::userAbstraction("0xabc");
    EXPECT_EQ(body["type"], "userAbstraction");
    EXPECT_EQ(body["user"], "0xabc");
}

// --- Response parser tests ---
//
// Both endpoints return a bare top-level JSON scalar (not wrapped in an object) - see
// @nktkas/hyperliquid for the confirmed shape, since these two aren't documented in the
// public gitbook docs beyond a page reference.

TEST(RestApiMessageParserInfoTest, ParseUserDexAbstractionStateTrue)
{
    RestApiMessageParser parser;
    auto response = parser.parseUserDexAbstractionState("true");

    ASSERT_TRUE(response.enabled.has_value());
    EXPECT_TRUE(*response.enabled);
}

TEST(RestApiMessageParserInfoTest, ParseUserDexAbstractionStateFalse)
{
    RestApiMessageParser parser;
    auto response = parser.parseUserDexAbstractionState("false");

    ASSERT_TRUE(response.enabled.has_value());
    EXPECT_FALSE(*response.enabled);
}

TEST(RestApiMessageParserInfoTest, ParseUserDexAbstractionStateNull)
{
    RestApiMessageParser parser;
    auto response = parser.parseUserDexAbstractionState("null");

    EXPECT_FALSE(response.enabled.has_value());
}

TEST(RestApiMessageParserInfoTest, ParseUserAbstractionUnifiedAccount)
{
    RestApiMessageParser parser;
    auto response = parser.parseUserAbstraction(R"("unifiedAccount")");

    EXPECT_EQ(response.state, UserAbstractionState::UnifiedAccount);
}

TEST(RestApiMessageParserInfoTest, ParseUserAbstractionPortfolioMargin)
{
    RestApiMessageParser parser;
    auto response = parser.parseUserAbstraction(R"("portfolioMargin")");

    EXPECT_EQ(response.state, UserAbstractionState::PortfolioMargin);
}

TEST(RestApiMessageParserInfoTest, ParseUserAbstractionDisabled)
{
    RestApiMessageParser parser;
    auto response = parser.parseUserAbstraction(R"("disabled")");

    EXPECT_EQ(response.state, UserAbstractionState::Disabled);
}

TEST(RestApiMessageParserInfoTest, ParseUserAbstractionDefault)
{
    RestApiMessageParser parser;
    auto response = parser.parseUserAbstraction(R"("default")");

    EXPECT_EQ(response.state, UserAbstractionState::Default);
}
