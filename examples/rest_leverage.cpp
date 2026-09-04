#include "test_config.h"

#include <hyperliquid/rest/RestApi.h>
#include <hyperliquid/config/Config.h>
#include <spdlog/spdlog.h>

void logSimpleResponse(const char* action, const hyperliquid::SimpleResponse& resp)
{
    spdlog::info("{}: status={} type={}", action, resp.status, resp.type);
    if (resp.error)
        spdlog::info("  Error: {}", *resp.error);
}

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

int main()
{
    auto wallet = loadWalletFromConfig();
    hyperliquid::setLogLevel(hyperliquid::LogLevel::Debug);

    hyperliquid::ApiConfig config;
    config.env = hyperliquid::Environment::Testnet;
    config.wallet = wallet;

    hyperliquid::RestApi api(config);

    // NOTE: these are signed, state-changing exchange actions - they persist on the
    // account (even on testnet) once executed. This example is intentionally not run
    // automatically; a human should run it deliberately.

    const std::string asset = "ETH";
    const double size = 0.01;

    spdlog::info("=== Switch {} to isolated leverage ===", asset);

    hyperliquid::UpdateLeverageRequest leverageReq;
    leverageReq.asset = asset;
    leverageReq.isCross = false;
    leverageReq.leverage = 10;

    auto leverageResp = api.updateLeverage(leverageReq);
    logSimpleResponse("Update leverage (isolated, 10x)", leverageResp);

    // updateIsolatedMargin needs an existing isolated position to act on, so open one
    // with an aggressive IOC order that crosses the book and fills immediately.
    spdlog::info("=== Open isolated position ({} {}) ===", size, asset);

    auto book = api.l2Book(asset);
    if (book.asks.empty())
    {
        spdlog::error("No asks in book for {}, aborting", asset);
        return 1;
    }
    double bestAsk = std::stod(book.asks.front().px);
    double aggressiveBuyPx = bestAsk * 1.01;

    hyperliquid::OrderRequest openOrder;
    openOrder.asset = asset;
    openOrder.isBuy = true;
    openOrder.price = aggressiveBuyPx;
    openOrder.size = size;
    openOrder.reduceOnly = false;
    openOrder.limit = hyperliquid::LimitOrderType{hyperliquid::Tif::Ioc};

    auto openResp = api.placeOrder({openOrder}, hyperliquid::Grouping::Na);
    logPlaceOrder(openResp);

    bool opened = false;
    for (const auto& s : openResp.statuses)
        if (s.filled) opened = true;

    if (!opened)
    {
        spdlog::warn("Open order did not fill (IOC missed the book) - skipping isolated margin update and close");
        return 0;
    }

    spdlog::info("=== Update isolated margin ===");

    hyperliquid::UpdateIsolatedMarginRequest marginReq;
    marginReq.asset = asset;
    marginReq.isBuy = true;
    marginReq.ntli = 1000000; // add $1.00 (6 decimals: 1,000,000 == 1 USD)

    auto marginResp = api.updateIsolatedMargin(marginReq);
    logSimpleResponse("Update isolated margin (+$1.00)", marginResp);

    // Close the position we opened above with an opposite reduceOnly IOC order.
    spdlog::info("=== Close isolated position ===");

    auto closeBook = api.l2Book(asset);
    if (closeBook.bids.empty())
    {
        spdlog::error("No bids in book for {}, cannot close - close manually", asset);
        return 1;
    }
    double bestBid = std::stod(closeBook.bids.front().px);
    double aggressiveSellPx = bestBid * 0.99;

    hyperliquid::OrderRequest closeOrder;
    closeOrder.asset = asset;
    closeOrder.isBuy = false;
    closeOrder.price = aggressiveSellPx;
    closeOrder.size = size;
    closeOrder.reduceOnly = true;
    closeOrder.limit = hyperliquid::LimitOrderType{hyperliquid::Tif::Ioc};

    logPlaceOrder(api.placeOrder({closeOrder}, hyperliquid::Grouping::Na));

    return 0;
}
