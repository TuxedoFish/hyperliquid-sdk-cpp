#include <gtest/gtest.h>

#include "messages/ExchangeRequestBuilder.h"
#include "hyperliquid/rest/RestApiMessageParser.h"

using namespace hyperliquid;

TEST(ScheduleCancelBuilder, ArmIncludesTimeField)
{
    ExchangeRequestBuilder builder;

    auto body = builder.scheduleCancel(1735689600000ULL);

    ASSERT_TRUE(body.contains("action"));
    const auto& action = body["action"];
    EXPECT_EQ(action["type"], "scheduleCancel");
    ASSERT_TRUE(action.contains("time"));
    EXPECT_EQ(action["time"].get<uint64_t>(), 1735689600000ULL);
}

TEST(ScheduleCancelBuilder, DisarmOmitsTimeField)
{
    ExchangeRequestBuilder builder;

    // No timestamp -> disarms any previously scheduled cancel-all.
    auto body = builder.scheduleCancel();

    ASSERT_TRUE(body.contains("action"));
    const auto& action = body["action"];
    EXPECT_EQ(action["type"], "scheduleCancel");
    EXPECT_FALSE(action.contains("time"));
}

TEST(ScheduleCancelBuilder, DisarmExplicitNulloptOmitsTimeField)
{
    ExchangeRequestBuilder builder;

    auto body = builder.scheduleCancel(std::nullopt);

    const auto& action = body["action"];
    EXPECT_FALSE(action.contains("time"));
}

TEST(ScheduleCancelResponseParsing, SuccessResponse)
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

TEST(ScheduleCancelResponseParsing, ErrorResponse)
{
    static const std::string kErr = R"({
        "status": "err",
        "response": "Cannot set scheduled cancel time until enough volume traded."
    })";

    RestApiMessageParser parser;
    auto resp = parser.parseSimpleResponse(kErr);

    EXPECT_EQ(resp.status, "err");
    ASSERT_TRUE(resp.error.has_value());
    EXPECT_EQ(*resp.error, "Cannot set scheduled cancel time until enough volume traded.");
}
