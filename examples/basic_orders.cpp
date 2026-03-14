#include "test_config.h"

#include <hyperliquid/rest/RestApi.h>
#include <hyperliquid/rest/RestApiMessageParser.h>
#include <hyperliquid/Logger.h>
#include <spdlog/spdlog.h>

int main()
{
    auto wallet = loadWalletFromConfig();

    hyperliquid::setLogLevel(hyperliquid::LogLevel::Debug);

    hyperliquid::RestApiConfig config;
    config.env = hyperliquid::Environment::Testnet;
    config.wallet = wallet;

    hyperliquid::RestApi api(config);
    hyperliquid::RestApiMessageParser parser;

    // =========================================================
    // Approach 1: Place and cancel by oid
    // =========================================================

    spdlog::info("=== Approach 1: Place and cancel by oid ===");

    hyperliquid::OrderRequest order1;
    order1.asset = "ETH";
    order1.isBuy = true;
    order1.price = 1800.0;
    order1.size = 0.01;
    order1.reduceOnly = false;
    order1.limit = hyperliquid::LimitOrderType{hyperliquid::Tif::Gtc};

    spdlog::info("Placing order...");
    auto placeRaw1 = api.placeOrder({order1}, hyperliquid::Grouping::Na);
    spdlog::info("Place response: {}", placeRaw1);
    auto placeResp1 = parser.parsePlaceOrder(placeRaw1);
    spdlog::info("Place order status: {}", placeResp1.status);

    uint64_t oid = 0;
    for (const auto& status : placeResp1.statuses)
    {
        if (status.resting)
        {
            oid = status.resting->oid;
            spdlog::info("  Resting oid={}", oid);
        }
        else if (status.filled)
        {
            oid = status.filled->oid;
            spdlog::info("  Filled oid={} avgPx={} totalSz={}", status.filled->oid, status.filled->avgPx, status.filled->totalSz);
        }
        else if (status.error)
        {
            spdlog::info("  Error: {}", *status.error);
        }
    }

    if (oid != 0)
    {
        spdlog::info("Cancelling order oid={}...", oid);

        hyperliquid::CancelRequest cancel1;
        cancel1.asset = "ETH";
        cancel1.oid = oid;

        auto cancelRaw1 = api.cancelOrder({cancel1});
        spdlog::info("Cancel response: {}", cancelRaw1);
        auto cancelResp1 = parser.parseCancelOrder(cancelRaw1);
        spdlog::info("Cancel order status: {}", cancelResp1.status);
        for (const auto& status : cancelResp1.statuses)
        {
            if (status.success)
                spdlog::info("  Success: {}", *status.success);
            else if (status.error)
                spdlog::info("  Error: {}", *status.error);
        }
    }

    // =========================================================
    // Approach 2: Place with cloid, modify, cancel by cloid
    // =========================================================

    spdlog::info("=== Approach 2: Place with cloid, modify, cancel by cloid ===");

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
    auto placeRaw2 = api.placeOrder({order2}, hyperliquid::Grouping::Na);
    spdlog::info("Place response: {}", placeRaw2);
    auto placeResp2 = parser.parsePlaceOrder(placeRaw2);
    spdlog::info("Place order status: {}", placeResp2.status);
    for (const auto& status : placeResp2.statuses)
    {
        if (status.resting)
            spdlog::info("  Resting oid={}", status.resting->oid);
        else if (status.filled)
            spdlog::info("  Filled oid={}", status.filled->oid);
        else if (status.error)
            spdlog::info("  Error: {}", *status.error);
    }

    spdlog::info("Modifying order (changing price to 1750.0)...");

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

    auto modifyRaw = api.modifyOrder(modify);
    spdlog::info("Modify response: {}", modifyRaw);
    auto modifyResp = parser.parseModifyOrder(modifyRaw);
    spdlog::info("Modify order status: {}", modifyResp.status);

    spdlog::info("Cancelling order by cloid={}...", cloid);

    hyperliquid::CancelByCloidRequest cancel2;
    cancel2.asset = "ETH";
    cancel2.cloid = cloid;

    auto cancelRaw2 = api.cancelOrderByCloid({cancel2});
    spdlog::info("Cancel response: {}", cancelRaw2);
    auto cancelResp2 = parser.parseCancelOrder(cancelRaw2);
    spdlog::info("Cancel order status: {}", cancelResp2.status);
    for (const auto& status : cancelResp2.statuses)
    {
        if (status.success)
            spdlog::info("  Success: {}", *status.success);
        else if (status.error)
            spdlog::info("  Error: {}", *status.error);
    }

    return 0;
}
