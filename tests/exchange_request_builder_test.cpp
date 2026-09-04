#include <gtest/gtest.h>

#include <stdexcept>

#include "messages/ExchangeRequestBuilder.h"

using namespace hyperliquid;

static OrderRequest makeLimitOrder(int assetId, bool isBuy, double price, double size, Tif tif = Tif::Gtc)
{
    OrderRequest order;
    order.asset = "ETH";
    order.assetId = assetId;
    order.isBuy = isBuy;
    order.price = price;
    order.size = size;
    order.reduceOnly = false;
    order.limit = LimitOrderType{tif};
    return order;
}

TEST(ExchangeRequestBuilderPlaceOrder, LimitOrderWireShape)
{
    ExchangeRequestBuilder builder;
    auto order = makeLimitOrder(4, true, 1670.1, 0.0147, Tif::Ioc);

    auto body = builder.placeOrder({order}, Grouping::Na);
    const auto& action = body["action"];
    EXPECT_EQ(action["type"], "order");
    EXPECT_EQ(action["grouping"], "na");
    ASSERT_EQ(action["orders"].size(), 1u);

    const auto& wire = action["orders"][0];
    EXPECT_EQ(wire["a"], 4);
    EXPECT_EQ(wire["b"], true);
    EXPECT_EQ(wire["p"], "1670.1");
    EXPECT_EQ(wire["s"], "0.0147");
    EXPECT_EQ(wire["r"], false);
    EXPECT_EQ(wire["t"]["limit"]["tif"], "Ioc");
    EXPECT_FALSE(wire.contains("c"));
}

TEST(ExchangeRequestBuilderPlaceOrder, TriggerOrderWireShape)
{
    ExchangeRequestBuilder builder;

    OrderRequest order;
    order.asset = "BTC";
    order.assetId = 0;
    order.isBuy = false;
    order.price = 100;
    order.size = 1;
    order.reduceOnly = true;
    order.trigger = TriggerOrderType{true, 103.0, TpSl::Sl};

    auto body = builder.placeOrder({order}, Grouping::PositionTpsl);
    const auto& wire = body["action"]["orders"][0];
    EXPECT_EQ(wire["r"], true);
    EXPECT_EQ(wire["t"]["trigger"]["isMarket"], true);
    EXPECT_EQ(wire["t"]["trigger"]["triggerPx"], "103");
    EXPECT_EQ(wire["t"]["trigger"]["tpsl"], "sl");
    EXPECT_EQ(body["action"]["grouping"], "positionTpsl");
}

TEST(ExchangeRequestBuilderPlaceOrder, IncludesCloidWhenPresent)
{
    ExchangeRequestBuilder builder;
    auto order = makeLimitOrder(4, true, 100, 100);
    order.cloid = "0x00000000000000000000000000000001";

    auto body = builder.placeOrder({order}, Grouping::Na);
    EXPECT_EQ(body["action"]["orders"][0]["c"], "0x00000000000000000000000000000001");
}

TEST(ExchangeRequestBuilderPlaceOrder, IncludesBuilderFeeWhenPresent)
{
    ExchangeRequestBuilder builder;
    auto order = makeLimitOrder(4, true, 100, 100);

    Builder fee;
    fee.address = "0x1719884eb866cb12b2287399b15f7db5e7d775ea";
    fee.fee = 10;

    auto body = builder.placeOrder({order}, Grouping::Na, fee);
    ASSERT_TRUE(body["action"].contains("builder"));
    EXPECT_EQ(body["action"]["builder"]["b"], "0x1719884eb866cb12b2287399b15f7db5e7d775ea");
    EXPECT_EQ(body["action"]["builder"]["f"], 10);
}

TEST(ExchangeRequestBuilderPlaceOrder, OmitsBuilderWhenAbsent)
{
    ExchangeRequestBuilder builder;
    auto order = makeLimitOrder(4, true, 100, 100);

    auto body = builder.placeOrder({order}, Grouping::Na);
    EXPECT_FALSE(body["action"].contains("builder"));
}

TEST(ExchangeRequestBuilderPlaceOrder, MultipleOrdersPreserveOrdering)
{
    ExchangeRequestBuilder builder;
    auto first = makeLimitOrder(0, true, 10, 1);
    auto second = makeLimitOrder(4, false, 20, 2);

    auto body = builder.placeOrder({first, second}, Grouping::NormalTpsl);
    const auto& orders = body["action"]["orders"];
    ASSERT_EQ(orders.size(), 2u);
    EXPECT_EQ(orders[0]["a"], 0);
    EXPECT_EQ(orders[1]["a"], 4);
    EXPECT_EQ(body["action"]["grouping"], "normalTpsl");
}

TEST(ExchangeRequestBuilderPlaceOrder, ResolvesUnknownSymbolThrows)
{
    ExchangeRequestBuilder builder;
    OrderRequest order;
    order.asset = "ETH";
    order.isBuy = true;
    order.price = 100;
    order.size = 1;
    order.reduceOnly = false;
    order.limit = LimitOrderType{Tif::Gtc};

    EXPECT_THROW(builder.placeOrder({order}, Grouping::Na), std::invalid_argument);
}

// --- floatToWire edge cases, exercised via price/size formatting ---

TEST(ExchangeRequestBuilderFloatToWire, ZeroFormatsAsBareZero)
{
    ExchangeRequestBuilder builder;
    auto order = makeLimitOrder(0, true, 0.0, 0.0);

    auto wire = builder.placeOrder({order}, Grouping::Na)["action"]["orders"][0];
    EXPECT_EQ(wire["p"], "0");
    EXPECT_EQ(wire["s"], "0");
}

TEST(ExchangeRequestBuilderFloatToWire, NegativePriceFormatsWithSign)
{
    ExchangeRequestBuilder builder;
    auto order = makeLimitOrder(0, true, -100.5, 0.00001);

    auto wire = builder.placeOrder({order}, Grouping::Na)["action"]["orders"][0];
    EXPECT_EQ(wire["p"], "-100.5");
    EXPECT_EQ(wire["s"], "0.00001");
}

TEST(ExchangeRequestBuilderFloatToWire, ExactEightDecimalsSurvives)
{
    ExchangeRequestBuilder builder;
    auto order = makeLimitOrder(0, true, 0.12345678, 1);

    auto wire = builder.placeOrder({order}, Grouping::Na)["action"]["orders"][0];
    EXPECT_EQ(wire["p"], "0.12345678");
}

TEST(ExchangeRequestBuilderFloatToWire, TrailingZerosStripped)
{
    ExchangeRequestBuilder builder;
    auto order = makeLimitOrder(0, true, 1670.10000000, 100.0);

    auto wire = builder.placeOrder({order}, Grouping::Na)["action"]["orders"][0];
    EXPECT_EQ(wire["p"], "1670.1");
    EXPECT_EQ(wire["s"], "100");
}

TEST(ExchangeRequestBuilderFloatToWire, ValueRequiringRoundingBeyondEightDecimalsThrows)
{
    ExchangeRequestBuilder builder;
    // 1/3 has infinite decimals; formatting to 8dp loses precision beyond the
    // function's rounding tolerance, so this must be rejected rather than silently
    // sending a mis-priced order.
    auto order = makeLimitOrder(0, true, 1.0 / 3.0, 1);

    EXPECT_THROW(builder.placeOrder({order}, Grouping::Na), std::invalid_argument);
}

// --- cancelOrder / cancelOrderByCloid ---

TEST(ExchangeRequestBuilderCancel, CancelOrderWireShape)
{
    ExchangeRequestBuilder builder;
    CancelRequest cancel{"ETH", 12345, 4};

    auto body = builder.cancelOrder({cancel});
    EXPECT_EQ(body["action"]["type"], "cancel");
    const auto& wire = body["action"]["cancels"][0];
    EXPECT_EQ(wire["a"], 4);
    EXPECT_EQ(wire["o"], 12345);
}

TEST(ExchangeRequestBuilderCancel, CancelOrderUnknownSymbolThrows)
{
    ExchangeRequestBuilder builder;
    CancelRequest cancel{"ETH", 1, std::nullopt};
    EXPECT_THROW(builder.cancelOrder({cancel}), std::invalid_argument);
}

TEST(ExchangeRequestBuilderCancel, CancelByCloidWireShape)
{
    ExchangeRequestBuilder builder;
    CancelByCloidRequest cancel{"BTC", "0x00000000000000000000000000000001", 0};

    auto body = builder.cancelOrderByCloid({cancel});
    EXPECT_EQ(body["action"]["type"], "cancelByCloid");
    const auto& wire = body["action"]["cancels"][0];
    EXPECT_EQ(wire["asset"], 0);
    EXPECT_EQ(wire["cloid"], "0x00000000000000000000000000000001");
}

// --- modifyOrder / batchModifyOrder ---

TEST(ExchangeRequestBuilderModify, UsesOidWhenPresent)
{
    ExchangeRequestBuilder builder;
    ModifyRequest modify;
    modify.oid = 42;
    modify.order = makeLimitOrder(4, true, 100, 1);

    auto body = builder.modifyOrder(modify);
    EXPECT_EQ(body["action"]["type"], "modify");
    EXPECT_EQ(body["action"]["oid"], 42);
    EXPECT_EQ(body["action"]["order"]["a"], 4);
}

TEST(ExchangeRequestBuilderModify, UsesCloidWhenOidAbsent)
{
    ExchangeRequestBuilder builder;
    ModifyRequest modify;
    modify.cloid = "0x00000000000000000000000000000002";
    modify.order = makeLimitOrder(4, true, 100, 1);

    auto body = builder.modifyOrder(modify);
    EXPECT_EQ(body["action"]["oid"], "0x00000000000000000000000000000002");
}

TEST(ExchangeRequestBuilderModify, ThrowsWhenNeitherOidNorCloidSet)
{
    ExchangeRequestBuilder builder;
    ModifyRequest modify;
    modify.order = makeLimitOrder(4, true, 100, 1);

    EXPECT_THROW(builder.modifyOrder(modify), std::invalid_argument);
}

TEST(ExchangeRequestBuilderModify, BatchModifyPreservesPerEntryIdentifiers)
{
    ExchangeRequestBuilder builder;

    ModifyRequest byOid;
    byOid.oid = 1;
    byOid.order = makeLimitOrder(0, true, 10, 1);

    ModifyRequest byCloid;
    byCloid.cloid = "0x00000000000000000000000000000003";
    byCloid.order = makeLimitOrder(4, false, 20, 2);

    auto body = builder.batchModifyOrder({byOid, byCloid});
    EXPECT_EQ(body["action"]["type"], "batchModify");
    const auto& modifies = body["action"]["modifies"];
    ASSERT_EQ(modifies.size(), 2u);
    EXPECT_EQ(modifies[0]["oid"], 1);
    EXPECT_EQ(modifies[1]["oid"], "0x00000000000000000000000000000003");
}
