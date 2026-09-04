#include "test_config.h"

#include <hyperliquid/rest/RestApi.h>
#include <hyperliquid/config/Config.h>
#include <spdlog/spdlog.h>

void logPlaceOrder(const hyperliquid::PlaceOrderResponse& resp)
{
    spdlog::info("Place order: status={}", resp.status);
    for (const auto& s : resp.statuses)
    {
        if (s.resting)
            spdlog::info("  Resting oid={}", s.resting->oid);
        else if (s.filled)
            spdlog::info("  Filled oid={} avgPx={} totalSz={}", s.filled->oid, s.filled->avgPx, s.filled->totalSz);
        else if (s.error)
            spdlog::info("  Error: {}", *s.error);
    }
}

void logCancelOrder(const hyperliquid::CancelOrderResponse& resp)
{
    spdlog::info("Cancel order: status={}", resp.status);
    for (const auto& s : resp.statuses)
    {
        if (s.success)
            spdlog::info("  Success: {}", *s.success);
        else if (s.error)
            spdlog::info("  Error: {}", *s.error);
    }
}

int main()
{
    auto wallet = loadWalletFromConfig();
    hyperliquid::setLogLevel(hyperliquid::LogLevel::Debug);

    hyperliquid::ApiConfig config;
    config.env = hyperliquid::Environment::Testnet;
    config.wallet = wallet;

    hyperliquid::RestApi api(config);

    // Place and cancel by oid
    spdlog::info("=== Place and cancel by oid ===");

    hyperliquid::OrderRequest order1;
    order1.asset = "ETH";
    order1.isBuy = true;
    order1.price = 1800.0;
    order1.size = 0.01;
    order1.reduceOnly = false;
    order1.limit = hyperliquid::LimitOrderType{hyperliquid::Tif::Gtc};

    spdlog::info("Placing order...");
    auto placeResp1 = api.placeOrder({order1}, hyperliquid::Grouping::Na);
    logPlaceOrder(placeResp1);

    uint64_t oid = 0;
    for (const auto& s : placeResp1.statuses)
    {
        if (s.resting) oid = s.resting->oid;
        else if (s.filled) oid = s.filled->oid;
    }

    if (oid != 0)
    {
        spdlog::info("Cancelling oid={}...", oid);
        hyperliquid::CancelRequest cancel1;
        cancel1.asset = "ETH";
        cancel1.oid = oid;
        logCancelOrder(api.cancelOrder({cancel1}));
    }

    // Place with cloid, modify, cancel by cloid
    spdlog::info("=== Place with cloid, modify, cancel by cloid ===");

    std::string cloid = hyperliquid::generateCloid();
    spdlog::info("Generated cloid: {}", cloid);

    hyperliquid::OrderRequest order2;
    order2.asset = "ETH";
    order2.isBuy = true;
    order2.price = 1800.0;
    order2.size = 0.01;
    order2.reduceOnly = false;
    order2.limit = hyperliquid::LimitOrderType{hyperliquid::Tif::Gtc};
    order2.cloid = cloid;

    spdlog::info("Placing order with cloid...");
    logPlaceOrder(api.placeOrder({order2}, hyperliquid::Grouping::Na));

    spdlog::info("Modifying order (price -> 1750.0)...");
    hyperliquid::OrderRequest modifiedOrder;
    modifiedOrder.asset = "ETH";
    modifiedOrder.isBuy = true;
    modifiedOrder.price = 1750.0;
    modifiedOrder.size = 0.01;
    modifiedOrder.reduceOnly = false;
    modifiedOrder.limit = hyperliquid::LimitOrderType{hyperliquid::Tif::Gtc};
    modifiedOrder.cloid = cloid;

    hyperliquid::ModifyRequest modify;
    modify.cloid = cloid;
    modify.order = modifiedOrder;

    auto modifyResp = api.modifyOrder(modify);
    spdlog::info("Modify order: status={}", modifyResp.status);

    spdlog::info("Cancelling by cloid={}...", cloid);
    hyperliquid::CancelByCloidRequest cancel2;
    cancel2.asset = "ETH";
    cancel2.cloid = cloid;
    logCancelOrder(api.cancelOrderByCloid({cancel2}));

    return 0;
}
