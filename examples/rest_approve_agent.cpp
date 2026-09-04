#include "test_config.h"

#include <hyperliquid/rest/RestApi.h>
#include <hyperliquid/config/Config.h>
#include <hyperliquid/signing/Keys.h>
#include <spdlog/spdlog.h>

// Demonstrates approving a new agent (API) wallet: generate a throwaway keypair locally,
// then authorize its address to trade on behalf of the main account via approveAgent.
//
// The printed private key below belongs to the freshly generated agent wallet, not the
// main account - it must be saved by the caller to actually use the agent afterwards.
// NOTE: this example is not executed as part of the build/test verification for this PR;
// approving an agent is a real, consequential action even on testnet.
int main()
{
    auto wallet = loadWalletFromConfig();
    hyperliquid::setLogLevel(hyperliquid::LogLevel::Debug);

    hyperliquid::ApiConfig config;
    config.env = hyperliquid::Environment::Testnet;
    config.wallet = wallet;

    hyperliquid::RestApi api(config);

    auto agentPrivateKey = hyperliquid::generateAgentPrivateKey();
    auto agentAddress = hyperliquid::privateKeyToAddress(agentPrivateKey);

    spdlog::info("Generated agent address: {}", agentAddress);
    spdlog::info("Generated agent private key (save this - it is not recoverable): {}", agentPrivateKey);

    hyperliquid::ApproveAgentRequest req;
    req.agentAddress = agentAddress;
    req.agentName = "example-agent";

    auto resp = api.approveAgent(req);
    spdlog::info("Approve agent: status={} type={}", resp.status, resp.type);
    if (resp.error)
        spdlog::info("  Error: {}", *resp.error);

    return 0;
}
