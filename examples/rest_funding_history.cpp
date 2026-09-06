#include "test_config.h"

#include <hyperliquid/rest/RestApi.h>
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

    uint64_t nowMs = static_cast<uint64_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());
    uint64_t sevenDaysMs = 7ULL * 24 * 60 * 60 * 1000;
    uint64_t startTime = nowMs - sevenDaysMs;

    spdlog::info("=== userFunding ===");
    auto funding = api.userFunding(userAddress, startTime);
    spdlog::info("{} funding ledger entries", funding.updates.size());
    for (const auto& entry : funding.updates)
        spdlog::info("  time={} hash={} type={} coin={} fundingRate={} szi={} usdc={}",
                     entry.time, entry.hash, hyperliquid::toString(entry.delta.type),
                     entry.delta.coin.value_or("?"), entry.delta.fundingRate.value_or(0.0),
                     entry.delta.szi.value_or(0.0), entry.delta.usdc.value_or(0.0));

    spdlog::info("=== userNonFundingLedgerUpdates ===");
    auto ledgerUpdates = api.userNonFundingLedgerUpdates(userAddress, startTime);
    spdlog::info("{} non-funding ledger entries", ledgerUpdates.updates.size());
    for (const auto& entry : ledgerUpdates.updates)
        spdlog::info("  time={} hash={} type={} usdc={}",
                     entry.time, entry.hash, hyperliquid::toString(entry.delta.type), entry.delta.usdc.value_or(0.0));

    spdlog::info("=== fundingHistory(\"ETH\") ===");
    auto history = api.fundingHistory("ETH", startTime);
    spdlog::info("{} funding history entries", history.history.size());
    for (const auto& entry : history.history)
        spdlog::info("  coin={} fundingRate={} premium={} time={}",
                     entry.coin, entry.fundingRate, entry.premium, entry.time);

    return 0;
}
