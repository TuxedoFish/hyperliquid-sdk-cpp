#include <hyperliquid/rest/RestApi.h>
#include <../include/hyperliquid/config/Config.h>
#include <spdlog/spdlog.h>

int main() {
    hyperliquid::setLogLevel(hyperliquid::LogLevel::Debug);

    hyperliquid::ApiConfig config;
    config.env = hyperliquid::Environment::Testnet;

    hyperliquid::RestApi api(config);

    spdlog::info("=== perpsAtOpenInterestCap (default dex) ===");
    auto atCap = api.perpsAtOpenInterestCap();
    for (const auto& coin : atCap.coins) {
        spdlog::info("  {}", coin);
    }
    if (atCap.coins.empty()) {
        spdlog::info("  (none - expected on testnet, where trading volume rarely pushes a coin to its open-interest cap)");
    }

    // The default dex has essentially no volume on testnet, so it almost never has anything at
    // cap - but some of the many HIP-3 test dexes do. "hyna" is a real, live example of this at
    // time of writing.
    spdlog::info("=== perpsAtOpenInterestCap (dex=hyna) ===");
    auto atCapHyna = api.perpsAtOpenInterestCap("hyna");
    for (const auto& coin : atCapHyna.coins) {
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

    spdlog::info("=== perpConciseAnnotations ===");
    auto conciseAnnotations = api.perpConciseAnnotations();
    for (const auto& entry : conciseAnnotations.annotations) {
        std::string keywords;
        for (const auto& kw : entry.keywords) {
            if (!keywords.empty()) keywords += ", ";
            keywords += kw;
        }
        spdlog::info("  {}  category={}  keywords=[{}]", entry.coin, entry.category, keywords);
    }

    spdlog::info("=== allPerpMetas ===");
    auto allMetas = api.allPerpMetas();
    spdlog::info("{} perp dexes", allMetas.dexMetas.size());
    for (const auto& dex : allMetas.dexMetas) {
        spdlog::info("  {} assets, {} margin tables", dex.universe.size(), dex.marginTables.size());
    }

    return 0;
}
