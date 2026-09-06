#include <hyperliquid/rest/RestApi.h>
#include <hyperliquid/config/Config.h>
#include <spdlog/spdlog.h>

int main() {
    hyperliquid::setLogLevel(hyperliquid::LogLevel::Debug);

    hyperliquid::ApiConfig config;
    config.env = hyperliquid::Environment::Testnet;

    hyperliquid::RestApi api(config);

    // Sad path: the main dex (empty string) has no HIP-3 deployer limits/status of its own.
    spdlog::info("=== perpDexLimits(\"\") - main dex ===");
    auto mainLimits = api.perpDexLimits("");
    spdlog::info("  exists={}", mainLimits.exists);

    // Happy path: "hyna" is a real, live HIP-3 dex at time of writing.
    spdlog::info("=== perpDexLimits(\"hyna\") ===");
    auto hynaLimits = api.perpDexLimits("hyna");
    spdlog::info("  exists={} totalOiCap={} oiSzCapPerPerp={} maxTransferNtl={}",
                 hynaLimits.exists, hynaLimits.totalOiCap, hynaLimits.oiSzCapPerPerp, hynaLimits.maxTransferNtl);
    for (const auto& cap : hynaLimits.coinToOiCap) {
        spdlog::info("  coin={} oiCap={}", cap.coin, cap.oiCap);
    }

    spdlog::info("=== perpDexStatus(\"\") - main dex ===");
    auto mainStatus = api.perpDexStatus("");
    spdlog::info("  exists={} totalNetDeposit={}", mainStatus.exists, mainStatus.totalNetDeposit);

    spdlog::info("=== perpDexStatus(\"hyna\") ===");
    auto hynaStatus = api.perpDexStatus("hyna");
    spdlog::info("  exists={} totalNetDeposit={}", hynaStatus.exists, hynaStatus.totalNetDeposit);

    // Takes no dex parameter - the deploy auction is global, not per-dex.
    spdlog::info("=== perpDeployAuctionStatus() ===");
    auto auctionStatus = api.perpDeployAuctionStatus();
    spdlog::info("  startTimeSeconds={} durationSeconds={} startGas={} currentGas={} endGas={}",
                 auctionStatus.startTimeSeconds, auctionStatus.durationSeconds, auctionStatus.startGas,
                 auctionStatus.currentGas ? std::to_string(*auctionStatus.currentGas) : "null",
                 auctionStatus.endGas ? std::to_string(*auctionStatus.endGas) : "null");

    return 0;
}
