// Demonstrates the read-only /info REST endpoints added for issue #4:
// l2Book, candleSnapshot, allMids, openOrders, orderStatus, userFills,
// userFillsByTime and clearinghouseState.
//
// These are all unauthenticated /info queries (no signing, no funds at risk),
// so this is safe to run against either Mainnet or Testnet. We use the wallet's
// address (never the private key) as the "user" for the account-scoped queries.

#include "test_config.h"

#include <hyperliquid/rest/RestApi.h>
#include <hyperliquid/rest/RestApiMessageParser.h>
#include <hyperliquid/config/Config.h>
#include <spdlog/spdlog.h>

#include <chrono>

int main()
{
    // Only the account address is used below - the private key is never read or logged.
    std::string userAddress = loadWalletFromConfig().accountAddress;

    hyperliquid::setLogLevel(hyperliquid::LogLevel::Info);

    hyperliquid::ApiConfig config;
    config.env = hyperliquid::Environment::Testnet;
    config.skipBuildingSymbolMap = true;

    hyperliquid::RestApi api(config);
    hyperliquid::RestApiMessageParser parser;

    spdlog::info("=== l2Book (BTC) ===");
    auto book = parser.parseL2Book(api.l2Book("BTC"));
    spdlog::info("coin={} time={} bids={} asks={}", book.coin, book.time, book.bids.size(), book.asks.size());
    for (size_t i = 0; i < book.bids.size() && i < 3; ++i)
        spdlog::info("  bid px={} sz={} n={}", book.bids[i].px, book.bids[i].sz, book.bids[i].n);
    for (size_t i = 0; i < book.asks.size() && i < 3; ++i)
        spdlog::info("  ask px={} sz={} n={}", book.asks[i].px, book.asks[i].sz, book.asks[i].n);

    spdlog::info("=== allMids ===");
    auto mids = parser.parseAllMids(api.allMids());
    spdlog::info("{} mids", mids.mids.size());

    uint64_t now = static_cast<uint64_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count());
    uint64_t oneHourAgo = now - 3600ULL * 1000ULL;

    spdlog::info("=== candleSnapshot (BTC, 15m) ===");
    auto candles = parser.parseCandleSnapshot(api.candleSnapshot("BTC", "15m", oneHourAgo, now));
    spdlog::info("{} candles", candles.candles.size());
    for (const auto& c : candles.candles)
        spdlog::info("  open={} close={} high={} low={} volume={}", c.open, c.close, c.high, c.low, c.volume);

    if (!userAddress.empty())
    {
        spdlog::info("=== openOrders ===");
        auto openOrders = parser.parseOpenOrders(api.openOrders(userAddress));
        spdlog::info("{} open orders", openOrders.orders.size());
        for (const auto& o : openOrders.orders)
            spdlog::info("  coin={} side={} limitPx={} sz={} oid={}", o.coin, o.side, o.limitPx, o.sz, o.oid);

        spdlog::info("=== userFills ===");
        auto fills = parser.parseUserFills(api.userFills(userAddress));
        spdlog::info("{} fills", fills.fills.size());

        spdlog::info("=== clearinghouseState ===");
        auto chState = parser.parseClearinghouseState(api.clearinghouseState(userAddress));
        spdlog::info("accountValue={} withdrawable={} positions={}",
                     chState.marginSummary.accountValue, chState.withdrawable, chState.assetPositions.size());
    }

    return 0;
}
