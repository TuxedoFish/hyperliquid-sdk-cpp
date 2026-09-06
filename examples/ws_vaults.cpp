#include <chrono>
#include <thread>

#include <spdlog/spdlog.h>

#include "test_config.h"
#include "hyperliquid/types/RequestTypes.h"
#include "hyperliquid/websocket/WebsocketApi.h"
#include "hyperliquid/websocket/WebsocketApiListener.h"

class PostResponseLogger : public hyperliquid::WebsocketApiListener
{
public:
    // Typed callbacks - demonstrate the new mechanism with real parsed fields instead of raw JSON.
    void onVaultDetailsPostResponse(const hyperliquid::VaultDetailsResponse& resp,
                                    std::optional<uint64_t> correlationId) override
    {
        spdlog::info("[vaultDetails] correlationId={} name={} leader={} apr={}",
                     correlationId.value_or(0), resp.name, resp.leader, resp.apr);
    }

    void onUserVaultEquitiesPostResponse(const hyperliquid::UserVaultEquitiesResponse& resp,
                                         std::optional<uint64_t> correlationId) override
    {
        spdlog::info("[userVaultEquities] correlationId={} equities={}",
                     correlationId.value_or(0), resp.equities.size());
    }

    void onPortfolioPostResponse(const hyperliquid::PortfolioResponse& resp,
                                 std::optional<uint64_t> correlationId) override
    {
        spdlog::info("[portfolio] correlationId={} periods={}", correlationId.value_or(0), resp.periods.size());
    }

    void onReferralPostResponse(const hyperliquid::ReferralResponse& resp,
                                std::optional<uint64_t> correlationId) override
    {
        spdlog::info("[referral] correlationId={} cumVlm={} unclaimedRewards={}",
                     correlationId.value_or(0), resp.cumVlm, resp.unclaimedRewards);
    }

    void onUserRolePostResponse(const hyperliquid::UserRoleResponse& resp,
                                std::optional<uint64_t> correlationId) override
    {
        spdlog::info("[userRole] correlationId={} role={}",
                     correlationId.value_or(0), hyperliquid::toString(resp.role));
    }

    // Generic fallback - still fires for every post response, typed or not.
    void onPostResponse(const std::string& rawJson, hyperliquid::RestEndpointType type,
                        std::optional<uint64_t> correlationId) override
    {
        spdlog::debug("[post response] type={} correlationId={} payload={}",
                     hyperliquid::toString(type), correlationId.value_or(0), rawJson);
    }

    void onConnected() override
    {
        spdlog::info("Connected");
    }

    void onDisconnected(bool hasError, const std::string& errMsg) override
    {
        spdlog::info("Disconnected: hasError={} errMsg={}", hasError, errMsg);
    }
};

int main()
{
    std::string userAddress = loadWalletFromConfig().accountAddress;
    hyperliquid::setLogLevel(hyperliquid::LogLevel::Info);

    hyperliquid::ApiConfig config;
    config.env = hyperliquid::Environment::Testnet;
    config.skipBuildingSymbolMap = true;

    PostResponseLogger logger;
    hyperliquid::WebsocketApi ws(config, logger);

    ws.start();
    std::this_thread::sleep_for(std::chrono::seconds(2));

    const std::string vaultAddress = "0x1719884eb866cb12b2287399b15f7db5e7d775ea";

    uint64_t correlationId = 1;
    auto wait = []() { std::this_thread::sleep_for(std::chrono::milliseconds(300)); };

    ws.vaultDetails(vaultAddress, std::nullopt, correlationId++); wait();
    ws.userVaultEquities(userAddress, correlationId++); wait();
    ws.portfolio(userAddress, correlationId++); wait();
    ws.referral(userAddress, correlationId++); wait();
    ws.userRole(userAddress, correlationId++); wait();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    ws.stop();
    return 0;
}
