#include "test_config.h"

#include <hyperliquid/rest/RestApi.h>
#include <hyperliquid/config/Config.h>
#include <spdlog/spdlog.h>

void logSimpleResponse(const char* action, const hyperliquid::SimpleResponse& resp)
{
    spdlog::info("{}: status={} type={}", action, resp.status, resp.type);
    if (resp.error)
        spdlog::info("  Error: {}", *resp.error);
}

int main()
{
    auto wallet = loadWalletFromConfig();
    std::string userAddress = wallet.accountAddress;
    hyperliquid::setLogLevel(hyperliquid::LogLevel::Debug);

    hyperliquid::ApiConfig config;
    config.env = hyperliquid::Environment::Testnet;
    config.wallet = wallet;

    hyperliquid::RestApi api(config);

    spdlog::info("=== delegatorSummary ===");
    auto summary = api.delegatorSummary(userAddress);
    spdlog::info("delegated={} undelegated={} totalPendingWithdrawal={} nPendingWithdrawals={}",
                 summary.delegated, summary.undelegated, summary.totalPendingWithdrawal, summary.nPendingWithdrawals);

    spdlog::info("=== delegations ===");
    auto delegations = api.delegations(userAddress);
    spdlog::info("{} delegations", delegations.delegations.size());
    for (const auto& d : delegations.delegations)
        spdlog::info("  validator={} amount={} lockedUntilTimestamp={}", d.validator, d.amount, d.lockedUntilTimestamp);

    spdlog::info("=== delegatorHistory ===");
    auto history = api.delegatorHistory(userAddress);
    spdlog::info("{} history entries", history.history.size());
    for (size_t i = 0; i < history.history.size() && i < 3; ++i)
        spdlog::info("  time={} hash={} deltaType={}",
                     history.history[i].time, history.history[i].hash,
                     hyperliquid::toString(history.history[i].delta.type));

    spdlog::info("=== delegatorRewards ===");
    auto rewards = api.delegatorRewards(userAddress);
    spdlog::info("{} reward entries", rewards.rewards.size());

    // cDeposit/cWithdraw/tokenDelegate are real staking exchange actions - this account has no
    // HYPE balance on testnet, so these are expected to fail with an insufficient-balance style
    // business-logic error. That's still useful evidence: it confirms the request is built,
    // signed, and submitted correctly (a malformed request would fail before reaching that check).
    spdlog::info("=== cDeposit (expect insufficient-balance rejection) ===");
    logSimpleResponse("cDeposit", api.cDeposit(1));

    spdlog::info("=== cWithdraw (expect insufficient-balance rejection) ===");
    logSimpleResponse("cWithdraw", api.cWithdraw(1));

    spdlog::info("=== tokenDelegate (expect insufficient-balance rejection) ===");
    hyperliquid::TokenDelegateRequest delegateReq;
    delegateReq.validator = "0x0000472d488d33b7329ca53bfcc3918961d55f8e"; // "Puffer Node", from validatorSummaries
    delegateReq.wei = 1;
    delegateReq.isUndelegate = false;
    logSimpleResponse("tokenDelegate", api.tokenDelegate(delegateReq));

    return 0;
}
