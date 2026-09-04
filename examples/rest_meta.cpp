#include <hyperliquid/rest/RestApi.h>
#include <../include/hyperliquid/config/Config.h>
#include <spdlog/spdlog.h>

int main() {
    hyperliquid::setLogLevel(hyperliquid::LogLevel::Debug);

    hyperliquid::ApiConfig config;
    config.env = hyperliquid::Environment::Mainnet;

    hyperliquid::RestApi api(config);

    spdlog::info("=== Spot ===");
    auto spotMeta = api.spotMeta();

    spdlog::info("{} assets:", spotMeta.tokens.size());
    for (const auto& asset : spotMeta.tokens) {
        spdlog::info("  {}  szDecimals={}", asset.name, asset.szDecimals);
    }

    auto dexes = api.perpDexs();

    spdlog::info("=== Outcomes ===");
    auto outcomeMeta = api.outcomeMeta();
    spdlog::info("{} outcomes:", outcomeMeta.outcomes.size());
    for (const auto& outcome : outcomeMeta.outcomes) {
        const auto& d = outcome.description;
        spdlog::info("  [{}] {}  class={} underlying={} expiry={} targetPrice={} period={}",
                     outcome.outcome, outcome.name, d.outcomeClass, d.underlying,
                     std::chrono::system_clock::to_time_t(d.expiry), d.targetPrice, d.period);
        for (const auto& side : outcome.sideSpecs) {
            spdlog::info("    side: {}", side.name);
        }
    }

    spdlog::info("=== Perps ===");
    auto defaultMeta = api.meta();

    spdlog::info("{} assets:", defaultMeta.universe.size());
    for (const auto& asset : defaultMeta.universe) {
        spdlog::info("  {}  szDecimals={}  maxLeverage={}", asset.name, asset.szDecimals, asset.maxLeverage);
    }

    spdlog::info("Found {} HIP-3 perp dexes:", dexes.dexes.size());
    for (const auto& dex : dexes.dexes) {
        spdlog::info("  {} ({})  deployer={}", dex.name, dex.fullName, dex.deployer);
    }

    for (const auto& dex : dexes.dexes) {
        spdlog::info("=== {} ===", dex.name);
        auto meta = api.meta(dex.name);

        spdlog::info("{} assets:", meta.universe.size());
        for (const auto& asset : meta.universe) {
            spdlog::info("  {}  szDecimals={}  maxLeverage={}", asset.name, asset.szDecimals, asset.maxLeverage);
        }
    }

    return 0;
}
