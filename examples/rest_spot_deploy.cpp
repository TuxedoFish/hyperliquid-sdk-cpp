#include <hyperliquid/rest/RestApi.h>
#include <hyperliquid/config/Config.h>
#include <spdlog/spdlog.h>

int main() {
    hyperliquid::setLogLevel(hyperliquid::LogLevel::Debug);

    hyperliquid::ApiConfig config;
    config.env = hyperliquid::Environment::Testnet;

    hyperliquid::RestApi api(config);

    spdlog::info("=== spotPairDeployAuctionStatus() ===");
    auto auctionStatus = api.spotPairDeployAuctionStatus();
    spdlog::info("  startTimeSeconds={} durationSeconds={} startGas={} currentGas={} endGas={}",
                 auctionStatus.startTimeSeconds, auctionStatus.durationSeconds, auctionStatus.startGas,
                 auctionStatus.currentGas ? std::to_string(*auctionStatus.currentGas) : "null",
                 auctionStatus.endGas ? std::to_string(*auctionStatus.endGas) : "null");

    // Sad path: a user address with no deployed spot tokens - "states" comes back empty.
    spdlog::info("=== spotDeployState(\"0x0000...0001\") - no deployed tokens ===");
    auto empty = api.spotDeployState("0x0000000000000000000000000000000000000001");
    spdlog::info("  states.size()={}", empty.states.size());

    // Happy path: a real testnet deployer address with a live spot token deployment.
    spdlog::info("=== spotDeployState(\"0x051d...46db\") - live deployment ===");
    auto deployed = api.spotDeployState("0x051dbfc562d44e4a01ebb986da35a47ab4f346db");
    for (const auto& state : deployed.states) {
        spdlog::info("  token={} name={} fullName={} maxSupply={} spots={}",
                     state.token, state.spec.name,
                     state.fullName ? *state.fullName : "null",
                     state.maxSupply ? std::to_string(*state.maxSupply) : "null",
                     state.spots.size());
        for (const auto& balance : state.userGenesisBalances) {
            spdlog::info("    userGenesisBalance address={} balance={}", balance.address, balance.balance);
        }
    }

    return 0;
}
