#include "test_config.h"

#include <hyperliquid/rest/RestApi.h>
#include <hyperliquid/config/Config.h>
#include <spdlog/spdlog.h>

int main()
{
    std::string userAddress = loadWalletFromConfig().accountAddress;

    hyperliquid::setLogLevel(hyperliquid::LogLevel::Info);

    hyperliquid::ApiConfig config;
    config.env = hyperliquid::Environment::Testnet;
    config.skipBuildingSymbolMap = true;

    hyperliquid::RestApi api(config);

    try
    {
        spdlog::info("=== userRateLimit ===");
        auto limit = api.userRateLimit(userAddress);
        spdlog::info("cumVlm={} nRequestsUsed={} nRequestsCap={} nRequestsSurplus={}",
                     limit.cumVlm, limit.nRequestsUsed, limit.nRequestsCap, limit.nRequestsSurplus);
    }
    catch (const hyperliquid::RestApiRateLimitError& e)
    {
        spdlog::error("Rate limited: {}", e.what());
        return 1;
    }
    catch (const hyperliquid::RestApiTransportError& e)
    {
        spdlog::error("Transport error: {}", e.what());
        return 1;
    }

    return 0;
}
