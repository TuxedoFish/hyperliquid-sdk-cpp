#include <gtest/gtest.h>

#include <unordered_map>

#include "hyperliquid/websocket/WebsocketApiListener.h"
#include "websocket/PostResponseDispatch.h"

using namespace hyperliquid;
using hyperliquid::internal::PostRequestInfo;

namespace
{
    struct CapturingListener : WebsocketApiListener
    {
        int genericCount = 0;
        std::vector<RestEndpointType> genericTypes;

        std::optional<L2BookResponse> l2Book;
        std::optional<AllMidsResponse> allMids;
        std::optional<PerpCategoriesResponse> perpCategories;
        std::optional<ClearinghouseState> clearinghouseState;

        int exchangeCount = 0;
        std::optional<RestEndpointType> exchangeType;
        std::optional<SimpleResponse> exchangeResponse;

        int onMessageCount = 0;

        void onPostResponse(const std::string&, RestEndpointType type,
                            std::optional<uint64_t>) override
        {
            genericCount++;
            genericTypes.push_back(type);
        }

        void onL2BookPostResponse(const L2BookResponse& resp, std::optional<uint64_t>) override
        {
            l2Book = resp;
        }

        void onAllMidsPostResponse(const AllMidsResponse& resp, std::optional<uint64_t>) override
        {
            allMids = resp;
        }

        void onPerpCategoriesPostResponse(const PerpCategoriesResponse& resp, std::optional<uint64_t>) override
        {
            perpCategories = resp;
        }

        void onClearinghouseStatePostResponse(const ClearinghouseState& resp, std::optional<uint64_t>) override
        {
            clearinghouseState = resp;
        }

        void onExchangeActionPostResponse(RestEndpointType type, const SimpleResponse& resp,
                                          std::optional<uint64_t>) override
        {
            exchangeCount++;
            exchangeType = type;
            exchangeResponse = resp;
        }

        void onMessage(const std::string&) override
        {
            onMessageCount++;
        }
    };

    std::string wrapPostMessage(uint64_t id, const std::string& responseType, const std::string& payloadJson)
    {
        return R"({"channel":"post","data":{"id":)" + std::to_string(id) +
               R"(,"response":{"type":")" + responseType + R"(","payload":)" + payloadJson + "}}}";
    }
}

TEST(WebsocketApiDispatch, L2BookTypedDispatchAndGenericFallback)
{
    CapturingListener listener;
    std::unordered_map<uint64_t, PostRequestInfo> postRequestInfo;
    postRequestInfo[1] = {RestEndpointType::L2Book, uint64_t{42}};

    const std::string payload = R"({"type":"l2Book","data":{"coin":"BTC","time":123,)"
                                 R"("levels":[[{"px":"100.0","sz":"1.0","n":1}],[{"px":"101.0","sz":"2.0","n":2}]]}})";
    const std::string raw = wrapPostMessage(1, "info", payload);

    internal::handlePostChannelMessage(raw, postRequestInfo, listener);

    ASSERT_TRUE(listener.l2Book.has_value());
    EXPECT_EQ(listener.l2Book->coin, "BTC");
    EXPECT_EQ(listener.l2Book->time, 123u);
    ASSERT_EQ(listener.l2Book->bids.size(), 1u);
    EXPECT_EQ(listener.l2Book->bids[0].px, "100.0");
    ASSERT_EQ(listener.l2Book->asks.size(), 1u);
    EXPECT_EQ(listener.l2Book->asks[0].px, "101.0");

    EXPECT_EQ(listener.genericCount, 1);
    EXPECT_EQ(listener.genericTypes[0], RestEndpointType::L2Book);
    EXPECT_TRUE(postRequestInfo.empty());
}

TEST(WebsocketApiDispatch, AllMidsTypedDispatchAndGenericFallback)
{
    CapturingListener listener;
    std::unordered_map<uint64_t, PostRequestInfo> postRequestInfo;
    postRequestInfo[2] = {RestEndpointType::AllMids, std::nullopt};

    const std::string payload = R"({"type":"allMids","data":{"BTC":"100000.0","ETH":"4000.0"}})";
    const std::string raw = wrapPostMessage(2, "info", payload);

    internal::handlePostChannelMessage(raw, postRequestInfo, listener);

    ASSERT_TRUE(listener.allMids.has_value());
    EXPECT_EQ(listener.allMids->mids.size(), 2u);
    EXPECT_EQ(listener.genericCount, 1);
}

TEST(WebsocketApiDispatch, PerpCategoriesTypedDispatchAndGenericFallback)
{
    CapturingListener listener;
    std::unordered_map<uint64_t, PostRequestInfo> postRequestInfo;
    postRequestInfo[3] = {RestEndpointType::PerpCategories, uint64_t{7}};

    const std::string payload = R"({"type":"perpCategories","data":[["BTC","layer1"],["ETH","layer1"]]})";
    const std::string raw = wrapPostMessage(3, "info", payload);

    internal::handlePostChannelMessage(raw, postRequestInfo, listener);

    ASSERT_TRUE(listener.perpCategories.has_value());
    ASSERT_EQ(listener.perpCategories->categories.size(), 2u);
    EXPECT_EQ(listener.perpCategories->categories[0].coin, "BTC");
    EXPECT_EQ(listener.perpCategories->categories[0].category, "layer1");
    EXPECT_EQ(listener.genericCount, 1);
}

TEST(WebsocketApiDispatch, ClearinghouseStateTypedDispatchAndGenericFallback)
{
    CapturingListener listener;
    std::unordered_map<uint64_t, PostRequestInfo> postRequestInfo;
    postRequestInfo[4] = {RestEndpointType::ClearinghouseState, std::nullopt};

    const std::string payload = R"({"type":"clearinghouseState","data":{)"
                                 R"("marginSummary":{"accountValue":"10.0","totalNtlPos":"1.0","totalRawUsd":"10.0","totalMarginUsed":"1.0"},)"
                                 R"("crossMarginSummary":{"accountValue":"10.0","totalNtlPos":"1.0","totalRawUsd":"10.0","totalMarginUsed":"1.0"},)"
                                 R"("crossMaintenanceMarginUsed":"0.5","withdrawable":"9.0","time":555,"assetPositions":[]}})";
    const std::string raw = wrapPostMessage(4, "info", payload);

    internal::handlePostChannelMessage(raw, postRequestInfo, listener);

    ASSERT_TRUE(listener.clearinghouseState.has_value());
    EXPECT_DOUBLE_EQ(listener.clearinghouseState->marginSummary.accountValue, 10.0);
    EXPECT_EQ(listener.clearinghouseState->time, 555u);
    EXPECT_EQ(listener.genericCount, 1);
}

TEST(WebsocketApiDispatch, ExchangeActionSuccessDispatchesSharedCallback)
{
    CapturingListener listener;
    std::unordered_map<uint64_t, PostRequestInfo> postRequestInfo;
    postRequestInfo[5] = {RestEndpointType::Noop, uint64_t{99}};

    const std::string payload = R"({"status":"ok","response":{"type":"default"}})";
    const std::string raw = wrapPostMessage(5, "action", payload);

    internal::handlePostChannelMessage(raw, postRequestInfo, listener);

    ASSERT_EQ(listener.exchangeCount, 1);
    ASSERT_TRUE(listener.exchangeType.has_value());
    EXPECT_EQ(*listener.exchangeType, RestEndpointType::Noop);
    ASSERT_TRUE(listener.exchangeResponse.has_value());
    EXPECT_EQ(listener.exchangeResponse->status, "ok");
    EXPECT_EQ(listener.exchangeResponse->type, "default");
    EXPECT_FALSE(listener.exchangeResponse->error.has_value());
    EXPECT_EQ(listener.genericCount, 1);
}

TEST(WebsocketApiDispatch, ExchangeActionErrorDispatchesSharedCallbackWithError)
{
    CapturingListener listener;
    std::unordered_map<uint64_t, PostRequestInfo> postRequestInfo;
    postRequestInfo[6] = {RestEndpointType::ReserveRequestWeight, std::nullopt};

    const std::string payload = R"({"status":"err","response":"Insufficient balance to reserve request weight."})";
    const std::string raw = wrapPostMessage(6, "action", payload);

    internal::handlePostChannelMessage(raw, postRequestInfo, listener);

    ASSERT_EQ(listener.exchangeCount, 1);
    EXPECT_EQ(*listener.exchangeType, RestEndpointType::ReserveRequestWeight);
    ASSERT_TRUE(listener.exchangeResponse.has_value());
    EXPECT_EQ(listener.exchangeResponse->status, "err");
    ASSERT_TRUE(listener.exchangeResponse->error.has_value());
    EXPECT_EQ(*listener.exchangeResponse->error, "Insufficient balance to reserve request weight.");
    EXPECT_EQ(listener.genericCount, 1);
}

TEST(WebsocketApiDispatch, UnknownIdFallsBackToOnMessageWithoutTypedOrGenericCallback)
{
    CapturingListener listener;
    std::unordered_map<uint64_t, PostRequestInfo> postRequestInfo;
    // Deliberately empty: id 123 was never recorded.

    const std::string payload = R"({"type":"l2Book","data":{"coin":"BTC","time":1,"levels":[[],[]]}})";
    const std::string raw = wrapPostMessage(123, "info", payload);

    internal::handlePostChannelMessage(raw, postRequestInfo, listener);

    EXPECT_EQ(listener.onMessageCount, 1);
    EXPECT_EQ(listener.genericCount, 0);
    EXPECT_FALSE(listener.l2Book.has_value());
}

// A malformed/unexpected typed payload (missing "data") must not crash and must not prevent the
// generic onPostResponse fallback from firing - it's the caller's safety net.
TEST(WebsocketApiDispatch, MalformedTypedPayloadIsLoggedNotThrownAndGenericStillFires)
{
    CapturingListener listener;
    std::unordered_map<uint64_t, PostRequestInfo> postRequestInfo;
    postRequestInfo[7] = {RestEndpointType::L2Book, std::nullopt};

    const std::string payload = R"({"type":"l2Book"})"; // missing "data"
    const std::string raw = wrapPostMessage(7, "info", payload);

    EXPECT_NO_THROW(internal::handlePostChannelMessage(raw, postRequestInfo, listener));

    EXPECT_FALSE(listener.l2Book.has_value());
    EXPECT_EQ(listener.genericCount, 1);
}
