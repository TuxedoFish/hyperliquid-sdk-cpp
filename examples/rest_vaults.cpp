#include "test_config.h"

#include <hyperliquid/rest/RestApi.h>
#include <hyperliquid/config/Config.h>
#include <spdlog/spdlog.h>

int main()
{
    // Only the account address is used below - the private key is never read or logged.
    std::string userAddress = loadWalletFromConfig().accountAddress;

    hyperliquid::setLogLevel(hyperliquid::LogLevel::Info);

    hyperliquid::ApiConfig config;
    config.env = hyperliquid::Environment::Testnet;
    config.skipBuildingSymbolMap = true;

    hyperliquid::RestApi api(config);

    spdlog::info("=== vaultDetails ===");
    // Same testnet vault used elsewhere in this repo's examples (rest_vault_transfer.cpp).
    std::string vaultAddress = "0x1719884eb866cb12b2287399b15f7db5e7d775ea";
    auto vault = api.vaultDetails(vaultAddress);
    spdlog::info("name={} leader={} apr={} isClosed={} followers={} portfolioPeriods={}",
                 vault.name, vault.leader, vault.apr, vault.isClosed,
                 vault.followers.size(), vault.portfolio.size());

    spdlog::info("=== userVaultEquities ===");
    auto equities = api.userVaultEquities(userAddress);
    spdlog::info("{} vault equities", equities.equities.size());
    for (const auto& e : equities.equities)
        spdlog::info("  vaultAddress={} equity={}", e.vaultAddress, e.equity);

    spdlog::info("=== portfolio ===");
    auto portfolio = api.portfolio(userAddress);
    spdlog::info("{} portfolio periods", portfolio.periods.size());
    for (const auto& p : portfolio.periods)
        spdlog::info("  period={} vlm={} historyPoints={}",
                     hyperliquid::toString(p.period), p.vlm, p.pnlHistory.size());

    spdlog::info("=== referral ===");
    auto referral = api.referral(userAddress);
    spdlog::info("referredBy={} cumVlm={} unclaimedRewards={} claimedRewards={}",
                 referral.referredBy.has_value(), referral.cumVlm, referral.unclaimedRewards, referral.claimedRewards);
    if (referral.referrerState)
        spdlog::info("  referrerState: stage={} required={}",
                     referral.referrerState->stage, referral.referrerState->required.value_or(-1));

    spdlog::info("=== userRole ===");
    auto role = api.userRole(userAddress);
    spdlog::info("role={}", hyperliquid::toString(role.role));

    return 0;
}
