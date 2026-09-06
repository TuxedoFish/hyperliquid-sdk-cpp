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
    // with a freshly-approved agent, and even after the account was already put into
    // UnifiedAccount mode via a successful userSetAbstraction call moments earlier - ruling out
    // signer approval and starting-state as the cause. The official TS SDK's own test suite only
    // exercises "u" here (its coverage check explicitly marks "i"/"p" as permanently
    // unsupported), so this is the one value expected to work at all - the remaining unknown is
    // some additional eligibility requirement on the agent-signed path specifically, since the
    // equivalent principal-signed userSetAbstraction("u") succeeds on this same zero-value
    // account. The request itself matches the official SDK's documented shape exactly.
    auto resp = agentApi.agentSetAbstraction(hyperliquid::UserAbstractionMode::UnifiedAccount);
    spdlog::info("agentSetAbstraction: status={} type={}", resp.status, resp.type);
    if (resp.error)
        spdlog::info("  Error: {}", *resp.error);

    return 0;
}
