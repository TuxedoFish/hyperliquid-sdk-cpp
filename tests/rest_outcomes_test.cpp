#include <gtest/gtest.h>

#include "messages/InfoRequestBuilder.h"
#include "hyperliquid/rest/RestApiMessageParser.h"

#include <nlohmann/json.hpp>

using namespace hyperliquid;

TEST(InfoRequestBuilderTest, SettledOutcome)
{
    auto body = InfoRequestBuilder::settledOutcome(10234);
    EXPECT_EQ(body["type"], "settledOutcome");
    EXPECT_EQ(body["outcome"], 10234);
    EXPECT_FALSE(isAuthenticated(RestEndpointType::SettledOutcome));
}

TEST(RestApiMessageParserInfoTest, ParseSettledOutcomeNull)
{
    // Never-settled/unknown outcome ids resolve to a bare JSON null.
    std::string message = "null";

    RestApiMessageParser parser;
    auto response = parser.parseSettledOutcome(message);

    EXPECT_FALSE(response.isSettled);
}

TEST(RestApiMessageParserInfoTest, ParseSettledOutcomeNoQuestion)
{
    std::string message =
        R"({"spec":{"outcome":1000,"name":"Recurring","description":"class:priceBinary|underlying:HYPE|expiry:20260313-0730|targetPrice:37.87|period:15m","sideSpecs":[{"name":"Yes"},{"name":"No"}],"quoteToken":"USDH"},"settleFraction":"0.0","details":"price:37.87"})";

    RestApiMessageParser parser;
    auto response = parser.parseSettledOutcome(message);

    EXPECT_TRUE(response.isSettled);
    EXPECT_EQ(response.spec.outcome, 1000);
    EXPECT_EQ(response.spec.name, "Recurring");
    EXPECT_EQ(response.spec.description.underlying, "HYPE");
    EXPECT_DOUBLE_EQ(response.settleFraction, 0.0);
    EXPECT_EQ(response.details, "price:37.87");
    EXPECT_FALSE(response.question.has_value());
}

TEST(RestApiMessageParserInfoTest, ParseSettledOutcomeQuestionSettled)
{
    // question.question is keyed "settled" (not "active") once the linked question itself
    // has resolved.
    std::string message =
        R"({"spec":{"outcome":10100,"name":"Recurring Named Outcome","description":"index:2","sideSpecs":[{"name":"Yes"},{"name":"No"}],"quoteToken":"USDC"},"settleFraction":"0.0","details":"price:2144.1","question":{"question":{"settled":796},"name":"Recurring","description":"class:priceBucket|underlying:ETH|expiry:20260521-0845|priceThresholds:2105,2190.9|period:15m"}})";

    RestApiMessageParser parser;
    auto response = parser.parseSettledOutcome(message);

    ASSERT_TRUE(response.question.has_value());
    EXPECT_TRUE(response.question->isSettled);
    EXPECT_EQ(response.question->questionId, 796);
    EXPECT_EQ(response.question->name, "Recurring");
}

TEST(RestApiMessageParserInfoTest, ParseSettledOutcomeQuestionActive)
{
    // question.question is keyed "active" while the linked question (e.g. an ongoing
    // multi-outcome tournament winner) has not yet resolved, even though this particular
    // outcome (one eliminated contestant) has already settled to "No".
    std::string message =
        R"({"spec":{"outcome":10234,"name":"Australia","description":"This outcome resolves to Yes if Australia is officially declared the 2026 HYPURR World Cup champion.","sideSpecs":[{"name":"Yes"},{"name":"No"}],"quoteToken":"USDC"},"settleFraction":"0.0","details":"Australia was eliminated from the 2026 FIFA World Cup and can no longer win the tournament.","question":{"question":{"active":823},"name":"2026 World Cup champion","description":"metadata=category:sports|subCategory:football"}})";

    RestApiMessageParser parser;
    auto response = parser.parseSettledOutcome(message);

    ASSERT_TRUE(response.question.has_value());
    EXPECT_FALSE(response.question->isSettled);
    EXPECT_EQ(response.question->questionId, 823);
    EXPECT_EQ(response.question->name, "2026 World Cup champion");
}
