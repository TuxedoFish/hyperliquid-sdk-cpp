#include <hyperliquid/rest/RestApi.h>
#include <../include/hyperliquid/config/Config.h>
#include <spdlog/spdlog.h>

int main() {
    hyperliquid::setLogLevel(hyperliquid::LogLevel::Debug);

    hyperliquid::ApiConfig config;
    config.env = hyperliquid::Environment::Testnet;

    hyperliquid::RestApi api(config);

    spdlog::info("=== perpsAtOpenInterestCap ===");
    auto atCap = api.perpsAtOpenInterestCap();
    for (const auto& coin : atCap.coins) {
        spdlog::info("  {}", coin);
    }

    spdlog::info("=== predictedFundings ===");
    auto fundings = api.predictedFundings();
    for (const auto& entry : fundings.fundings) {
        spdlog::info("{}:", entry.coin);
        for (const auto& venue : entry.venues) {
            if (venue.fundingRate) {
                spdlog::info("  {}  fundingRate={}  nextFundingTime={}",
                             venue.venue, *venue.fundingRate, venue.nextFundingTime.value_or(0));
            } else {
                spdlog::info("  {}  (no data)", venue.venue);
            }
        }
    }

    spdlog::info("=== perpCategories ===");
    auto categories = api.perpCategories();
    for (const auto& entry : categories.categories) {
        spdlog::info("  {}  ->  {}", entry.coin, entry.category);
    }

    spdlog::info("=== perpAnnotation(BTC) ===");
    auto annotation = api.perpAnnotation("BTC");
    spdlog::info("  category={}  description={}", annotation.category, annotation.description);

    spdlog::info("=== allPerpMetas ===");
    auto allMetas = api.allPerpMetas();
    spdlog::info("{} perp dexes", allMetas.dexMetas.size());
    for (const auto& dex : allMetas.dexMetas) {
        spdlog::info("  {} assets, {} margin tables", dex.universe.size(), dex.marginTables.size());
    }

    return 0;
}
