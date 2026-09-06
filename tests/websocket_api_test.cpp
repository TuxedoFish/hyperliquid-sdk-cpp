#include <gtest/gtest.h>

#include "hyperliquid/websocket/WebsocketApi.h"
#include "hyperliquid/websocket/WebsocketApiListener.h"
#include "messages/InfoRequestBuilder.h"

using namespace hyperliquid;

// WebsocketApi::x(...) is a thin wrapper: it just forwards InfoRequestBuilder::x(...)'s
// already-tested payload into the generic, already-tested Impl::signAndSend(RestEndpointType, ...)
// envelope (`{"method":"post","id":N,"request":{"type":"info","payload":{...}}}`). There's no
// live/mocked websocket harness in this repo to assert on bytes actually written to the socket,
// so these tests cover the two things that matter for a wrapper this thin:
//   1. The InfoRequestBuilder payload each new wrapper forwards has the expected shape.
//   2. The new WebsocketApi::x(...) methods compile, link, and can be invoked without crashing.

TEST(InfoRequestBuilderPayloads, SettledOutcome)
{
    auto body = InfoRequestBuilder::settledOutcome(1000);
    EXPECT_EQ(body["type"], "settledOutcome");
    EXPECT_EQ(body["outcome"], 1000);
}

TEST(InfoRequestBuilderPayloads, L2BookWithOptionalParams)
{
    auto body = InfoRequestBuilder::l2Book("BTC", 5, 2);
    EXPECT_EQ(body["type"], "l2Book");
    EXPECT_EQ(body["coin"], "BTC");
    EXPECT_EQ(body["nSigFigs"], 5);
    EXPECT_EQ(body["mantissa"], 2);
}

TEST(InfoRequestBuilderPayloads, ClearinghouseStateWithDex)
{
    auto body = InfoRequestBuilder::clearinghouseState("0xabc", "hyna");
    EXPECT_EQ(body["type"], "clearinghouseState");
    EXPECT_EQ(body["user"], "0xabc");
    EXPECT_EQ(body["dex"], "hyna");
    EXPECT_FALSE(InfoRequestBuilder::clearinghouseState("0xabc").contains("dex"));
}

TEST(InfoRequestBuilderPayloads, VaultDetailsOptionalUser)
{
    auto body = InfoRequestBuilder::vaultDetails("0xvault", "0xuser");
    EXPECT_EQ(body["type"], "vaultDetails");
    EXPECT_EQ(body["vaultAddress"], "0xvault");
    EXPECT_EQ(body["user"], "0xuser");
    EXPECT_FALSE(InfoRequestBuilder::vaultDetails("0xvault").contains("user"));
}

TEST(InfoRequestBuilderPayloads, BorrowLendReserveState)
{
    auto body = InfoRequestBuilder::borrowLendReserveState(0);
    EXPECT_EQ(body["type"], "borrowLendReserveState");
    EXPECT_EQ(body["token"], 0);
}

namespace
{
    struct NoopListener : WebsocketApiListener
    {
    };
}

// Constructing WebsocketApi with no configured wallet skips all network activity (no symbol map
// fetch), and calling the new wrappers without start()-ing the connection is safe: send() just
// posts the outbound message onto an io_context that is never run, so nothing is actually
// dispatched over the network. This is enough to prove the 43 backfilled methods compile, link,
// and don't throw when invoked - the main risk for a purely mechanical wrapper backfill like this.
TEST(WebsocketApiInfoWrappers, CompileLinkAndInvokeWithoutCrashing)
{
    ApiConfig config;
    config.env = Environment::Testnet;
    NoopListener listener;
    WebsocketApi ws(config, listener);

    EXPECT_NO_THROW(ws.settledOutcome(1000));
    EXPECT_NO_THROW(ws.perpsAtOpenInterestCap(std::nullopt));
    EXPECT_NO_THROW(ws.predictedFundings());
    EXPECT_NO_THROW(ws.perpAnnotation("BTC"));
    EXPECT_NO_THROW(ws.l2Book("BTC"));
    EXPECT_NO_THROW(ws.candleSnapshot("BTC", "15m", 0, 1));
    EXPECT_NO_THROW(ws.allMids());
    EXPECT_NO_THROW(ws.openOrders("0xabc"));
    EXPECT_NO_THROW(ws.orderStatus("0xabc", OrderId{uint64_t{1}}));
    EXPECT_NO_THROW(ws.userFills("0xabc"));
    EXPECT_NO_THROW(ws.userFillsByTime("0xabc", 0));
    EXPECT_NO_THROW(ws.clearinghouseState("0xabc"));
    EXPECT_NO_THROW(ws.userRateLimit("0xabc"));
    EXPECT_NO_THROW(ws.metaAndAssetCtxs());
    EXPECT_NO_THROW(ws.spotMetaAndAssetCtxs());
    EXPECT_NO_THROW(ws.spotClearinghouseState("0xabc"));
    EXPECT_NO_THROW(ws.spotDeployState("0xabc"));
    EXPECT_NO_THROW(ws.spotPairDeployAuctionStatus());
    EXPECT_NO_THROW(ws.frontendOpenOrders("0xabc"));
    EXPECT_NO_THROW(ws.historicalOrders("0xabc"));
    EXPECT_NO_THROW(ws.userTwapSliceFills("0xabc"));
    EXPECT_NO_THROW(ws.subAccounts("0xabc"));
    EXPECT_NO_THROW(ws.userFees("0xabc"));
    EXPECT_NO_THROW(ws.maxBuilderFee("0xabc", "0xbuilder"));
    EXPECT_NO_THROW(ws.approvedBuilders("0xabc"));
    EXPECT_NO_THROW(ws.delegations("0xabc"));
    EXPECT_NO_THROW(ws.delegatorSummary("0xabc"));
    EXPECT_NO_THROW(ws.delegatorHistory("0xabc"));
    EXPECT_NO_THROW(ws.delegatorRewards("0xabc"));
    EXPECT_NO_THROW(ws.vaultDetails("0xvault"));
    EXPECT_NO_THROW(ws.userVaultEquities("0xabc"));
    EXPECT_NO_THROW(ws.portfolio("0xabc"));
    EXPECT_NO_THROW(ws.referral("0xabc"));
    EXPECT_NO_THROW(ws.userRole("0xabc"));
    EXPECT_NO_THROW(ws.borrowLendUserState("0xabc"));
    EXPECT_NO_THROW(ws.borrowLendReserveState(0));
    EXPECT_NO_THROW(ws.allBorrowLendReserveStates());
    EXPECT_NO_THROW(ws.perpCategories());
    EXPECT_NO_THROW(ws.perpConciseAnnotations());
    EXPECT_NO_THROW(ws.allPerpMetas());
    EXPECT_NO_THROW(ws.perpDexLimits("hyna"));
    EXPECT_NO_THROW(ws.perpDexStatus("hyna"));
    EXPECT_NO_THROW(ws.perpDeployAuctionStatus());
}
