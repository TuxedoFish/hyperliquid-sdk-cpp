#include "test_config.h"

#include <hyperliquid/rest/RestApi.h>
#include <hyperliquid/config/Config.h>
#include <spdlog/spdlog.h>

int main()
{
    // agentSetAbstraction is an L1 action signed by the agent wallet itself (not the main
    // wallet), and changes an account-wide margin mode - config.wallet here must hold an agent
    // key already approved against the target account via approveAgent. Switching abstraction
    // mode affects margin computation for the whole account, and not all transitions are
    // reversible, so run this deliberately rather than as a routine smoke test.
    auto wallet = loadWalletFromConfig();
    hyperliquid::setLogLevel(hyperliquid::LogLevel::Debug);

    hyperliquid::ApiConfig config;
    config.env = hyperliquid::Environment::Testnet;
    config.wallet = wallet;

    hyperliquid::RestApi api(config);

    auto resp = api.agentSetAbstraction(hyperliquid::UserAbstractionMode::UnifiedAccount);
    spdlog::info("agentSetAbstraction: status={} type={}", resp.status, resp.type);
    if (resp.error)
        spdlog::info("  Error: {}", *resp.error);

    return 0;
}
