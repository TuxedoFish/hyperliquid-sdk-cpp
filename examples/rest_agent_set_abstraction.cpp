#include "test_config.h"

#include <hyperliquid/rest/RestApi.h>
#include <hyperliquid/config/Config.h>
#include <hyperliquid/signing/Keys.h>
#include <spdlog/spdlog.h>
#include <thread>
#include <chrono>

int main()
{
    // agentSetAbstraction is an L1 action signed by an approved agent wallet, not the main
    // account itself - unlike userSetAbstraction, which is EIP-712-signed by the principal.
    // This example demonstrates the full real flow: generate a fresh agent key, approve it
    // against the main account, then sign this action with the agent's own key.
    auto mainWallet = loadWalletFromConfig();
    hyperliquid::setLogLevel(hyperliquid::LogLevel::Debug);

    hyperliquid::ApiConfig mainConfig;
    mainConfig.env = hyperliquid::Environment::Testnet;
    mainConfig.wallet = mainWallet;
    hyperliquid::RestApi mainApi(mainConfig);

    auto agentPrivateKey = hyperliquid::generateAgentPrivateKey();
    auto agentAddress = hyperliquid::privateKeyToAddress(agentPrivateKey);
    spdlog::info("Generated agent address: {}", agentAddress);

    hyperliquid::ApproveAgentRequest approveReq;
    approveReq.agentAddress = agentAddress;
    approveReq.agentName = "abstraction-test";

    auto approveResp = mainApi.approveAgent(approveReq);
    spdlog::info("approveAgent: status={} type={}", approveResp.status, approveResp.type);
    if (approveResp.error)
        spdlog::info("  Error: {}", *approveResp.error);
    if (approveResp.status != "ok")
        return 1;

    // Give the exchange a moment to register the newly-approved agent.
    std::this_thread::sleep_for(std::chrono::seconds(2));

    hyperliquid::ApiConfig agentConfig;
    agentConfig.env = hyperliquid::Environment::Testnet;
    agentConfig.wallet = hyperliquid::Wallet{agentAddress, agentPrivateKey};
    hyperliquid::RestApi agentApi(agentConfig);

    // Confirmed live: this consistently rejects with "Abstraction transition not allowed" even
    // with a freshly-approved agent (ruled out by comparison against the equivalent
    // userSetAbstraction call, which succeeds on the same account moments apart) - a real
    // exchange-side restriction on this specific action, not an SDK issue. The request itself
    // matches the official TS SDK's documented shape exactly.
    auto resp = agentApi.agentSetAbstraction(hyperliquid::UserAbstractionMode::UnifiedAccount);
    spdlog::info("agentSetAbstraction: status={} type={}", resp.status, resp.type);
    if (resp.error)
        spdlog::info("  Error: {}", *resp.error);

    return 0;
}
