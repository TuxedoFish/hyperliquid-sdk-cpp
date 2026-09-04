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

    // This account holds real testnet HYPE (weiDecimals=8), so cDeposit/tokenDelegate below are
    // exercised as genuine successes, not just correctly-signed rejections.
    spdlog::info("=== cDeposit: 3 HYPE into staking ===");
    logSimpleResponse("cDeposit", api.cDeposit(3ULL * 100000000ULL));

    spdlog::info("=== delegatorSummary after deposit ===");
    summary = api.delegatorSummary(userAddress);
    spdlog::info("delegated={} undelegated={} totalPendingWithdrawal={} nPendingWithdrawals={}",
                 summary.delegated, summary.undelegated, summary.totalPendingWithdrawal, summary.nPendingWithdrawals);

    spdlog::info("=== tokenDelegate: delegate 2 HYPE to validator ===");
    hyperliquid::TokenDelegateRequest delegateReq;
    delegateReq.validator = "0x0000472d488d33b7329ca53bfcc3918961d55f8e"; // "Puffer Node", from validatorSummaries
    delegateReq.wei = 2ULL * 100000000ULL;
    delegateReq.isUndelegate = false;
    logSimpleResponse("tokenDelegate", api.tokenDelegate(delegateReq));

    spdlog::info("=== delegations after delegating ===");
    delegations = api.delegations(userAddress);
    spdlog::info("{} delegations", delegations.delegations.size());
    for (const auto& d : delegations.delegations)
        spdlog::info("  validator={} amount={} lockedUntilTimestamp={}", d.validator, d.amount, d.lockedUntilTimestamp);

    // Delegations have a lock-up before they can be undelegated, and cWithdraw (staking -> spot)
    // has a separate multi-day unstaking queue - so this is expected to be rejected on a
    // freshly-delegated balance. Still useful evidence: it confirms the request reaches the
    // correct, specific business-logic check rather than failing at signing/transport.
    spdlog::info("=== cWithdraw: 1 HYPE (expect lock-up/queue rejection) ===");
    logSimpleResponse("cWithdraw", api.cWithdraw(1ULL * 100000000ULL));

    return 0;
}
