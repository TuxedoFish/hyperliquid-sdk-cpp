#include <hyperliquid/rest/RestApi.h>
#include <hyperliquid/config/Config.h>
#include <spdlog/spdlog.h>

int main() {
    hyperliquid::setLogLevel(hyperliquid::LogLevel::Debug);

    hyperliquid::ApiConfig config;
    config.env = hyperliquid::Environment::Testnet;

    hyperliquid::RestApi api(config);

    spdlog::info("=== recentTrades(\"BTC\") ===");
    auto trades = api.recentTrades("BTC");
    spdlog::info("  trades.size()={}", trades.trades.size());
    for (const auto& trade : trades.trades) {
        spdlog::info("  coin={} side={} px={} sz={} time={} tid={} hash={} buyer={} seller={}",
                     trade.coin, trade.side, trade.px, trade.sz, trade.time, trade.tid, trade.hash,
                     trade.users[0], trade.users[1]);
    }

    return 0;
}
