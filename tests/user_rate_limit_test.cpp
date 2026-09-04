#include <gtest/gtest.h>

#include "messages/InfoRequestBuilder.h"
#include "hyperliquid/rest/RestApi.h"
#include "hyperliquid/rest/RestApiMessageParser.h"

using namespace hyperliquid;

TEST(InfoRequestBuilderTest, UserRateLimit)
{
    auto body = InfoRequestBuilder::userRateLimit("0xabc");
    EXPECT_EQ(body["type"], "userRateLimit");
    EXPECT_EQ(body["user"], "0xabc");
}

TEST(RestApiMessageParserInfoTest, ParseUserRateLimit)
{
    std::string message = R"({"cumVlm":"129563528.5100000054","nRequestsUsed":0,"nRequestsCap":129573528,"nRequestsSurplus":0})";

    RestApiMessageParser parser;
    auto limit = parser.parseUserRateLimit(message);

    EXPECT_DOUBLE_EQ(limit.cumVlm, 129563528.5100000054);
    EXPECT_EQ(limit.nRequestsUsed, 0);
    EXPECT_EQ(limit.nRequestsCap, 129573528);
    EXPECT_EQ(limit.nRequestsSurplus, 0);
}

TEST(RestApiMessageParserInfoTest, ParseUserRateLimitWithSurplus)
{
    std::string message = R"({"cumVlm":"1000.5","nRequestsUsed":250,"nRequestsCap":1000,"nRequestsSurplus":750})";

    RestApiMessageParser parser;
    auto limit = parser.parseUserRateLimit(message);

    EXPECT_DOUBLE_EQ(limit.cumVlm, 1000.5);
    EXPECT_EQ(limit.nRequestsUsed, 250);
    EXPECT_EQ(limit.nRequestsCap, 1000);
    EXPECT_EQ(limit.nRequestsSurplus, 750);
}

TEST(RestApiRateLimitErrorTest, IsATransportError)
{
    RestApiRateLimitError err("RestApi: HTTP 429 rate limited: {}");
    const RestApiTransportError& asBase = err;
    EXPECT_STREQ(asBase.what(), "RestApi: HTTP 429 rate limited: {}");
}
