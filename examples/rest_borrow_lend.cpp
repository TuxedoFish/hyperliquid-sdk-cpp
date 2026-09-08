#include "test_config.h"

#include <hyperliquid/rest/RestApi.h>
#include <hyperliquid/config/Config.h>
#include <spdlog/spdlog.h>

#include <chrono>

int main() {
    auto wallet = loadWalletFromConfig();
    hyperliquid::setLogLevel(hyperliquid::LogLevel::Debug);

    hyperliquid::ApiConfig config;
    config.env = hyperliquid::Environment::Testnet;
    config.wallet = wallet;

    hyperliquid::RestApi api(config);

    spdlog::info("=== allBorrowLendReserveStates ===");
    auto reserves = api.allBorrowLendReserveStates();
    spdlog::info("{} reserves", reserves.reserves.size());
    for (const auto& entry : reserves.reserves) {
        spdlog::info("  token={} totalSupplied={} totalBorrowed={} utilization={} ltv={}",
                     entry.token, entry.state.totalSupplied, entry.state.totalBorrowed,
                     entry.state.utilization, entry.state.ltv);
    }

    spdlog::info("=== borrowLendReserveState(token=0) ===");
    auto reserve0 = api.borrowLendReserveState(0);
    spdlog::info("  borrowYearlyRate={} supplyYearlyRate={} balance={} oraclePx={}",
                 reserve0.borrowYearlyRate, reserve0.supplyYearlyRate, reserve0.balance, reserve0.oraclePx);

    spdlog::info("=== borrowLendUserState(own wallet) ===");
    auto userState = api.borrowLendUserState(wallet.accountAddress);
    spdlog::info("  health={} healthFactor={} positions={}",
                 userState.health,
                 userState.healthFactor ? std::to_string(*userState.healthFactor) : "null",
                 userState.tokenToState.size());
    for (const auto& position : userState.tokenToState) {
        spdlog::info("  token={} supply.value={} borrow.value={}",
                     position.token, position.supply.value, position.borrow.value);
    }

    spdlog::info("=== userBorrowLendInterest(own wallet, last 30d) ===");
    uint64_t now = static_cast<uint64_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count());
    uint64_t thirtyDaysAgo = now - 30ULL * 24ULL * 3600ULL * 1000ULL;
    auto interest = api.userBorrowLendInterest(wallet.accountAddress, thirtyDaysAgo);
    spdlog::info("{} interest entries", interest.interest.size());
    for (const auto& entry : interest.interest) {
        spdlog::info("  time={} token={} borrow={} supply={}",
                     entry.time, entry.token, entry.borrow, entry.supply);
    }

    return 0;
}
