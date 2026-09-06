#include <chrono>
#include <thread>

#include <spdlog/spdlog.h>

#include "test_config.h"
#include "hyperliquid/websocket/WebsocketApi.h"
#include "hyperliquid/websocket/WebsocketApiListener.h"

// Smoke-tests the 23 backfilled WebsocketApi exchange-action wrappers against a real connection,
// one call at a time, so any wiring bug (wrong RestEndpointType, wrong parameter forwarding, wrong
// ExchangeRequestBuilder call) shows up as a real exchange response rather than only as
// compiling/unit-test success. Parameter values are copied from the existing, already-verified
// REST examples (rest_transfers.cpp, rest_staking.cpp, rest_twap.cpp, rest_leverage.cpp,
// rest_vault_transfer.cpp, rest_hip3_liquidator_transfer.cpp, rest_approve_agent.cpp,
// rest_approve_builder_fee.cpp, rest_agent_set_abstraction.cpp, rest_user_set_abstraction.cpp,
// rest_user_dex_abstraction.cpp, rest_misc_actions.cpp) rather than invented from scratch - several
// of these actions move real (testnet) funds or change real account state.
//
// This example only sends requests; it does not attempt to correlate/parse each response into a
// typed struct (that plumbing is unchanged - callers dispatch via RestApiMessageParser themselves,
// see ws_orders.cpp for that pattern). It logs the raw JSON payload of every response instead, so
// the person running this can eyeball real output per call.
//
// NOTE: intentionally not run by CI/automation - move real testnet funds/state, must be run
// manually, one call at a time, watching the log after each.

class PostLoggingListener : public hyperliquid::WebsocketApiListener
{
public:
    void onPostResponse(const std::string& rawJson, hyperliquid::RestEndpointType type,
                        std::optional<uint64_t> correlationId) override
    {
        spdlog::info("post response: type={} correlationId={} payload={}",
                     hyperliquid::toString(type),
                     correlationId ? std::to_string(*correlationId) : "none",
                     rawJson);
    }

    void onConnected() override
    {
        spdlog::info("Connected.");
    }

    void onDisconnected(bool hasError, const std::string& errMsg) override
    {
        spdlog::info("Disconnected: hasError={} errMsg={}", hasError, errMsg);
    }
};

namespace
{
    // Pause between each send so responses can be told apart in the log by eye.
    void betweenCallsDelay()
    {
        std::this_thread::sleep_for(std::chrono::seconds(3));
    }
}

int main()
{
    auto wallet = loadWalletFromConfig();
    hyperliquid::setLogLevel(hyperliquid::LogLevel::Debug);

    hyperliquid::ApiConfig config;
    config.env = hyperliquid::Environment::Testnet;
    config.wallet = wallet;

    PostLoggingListener listener;
    hyperliquid::WebsocketApi ws(config, listener);

    spdlog::info("Starting websocket...");
    ws.start();
    // Give the connection a moment to establish (and the symbol map to build) before sending.
    std::this_thread::sleep_for(std::chrono::seconds(2));

    const std::string destination = "0x5e9ee1089755c3435139848e47e6635505d5a13a";
    const std::string tokenId = "USDC:0xeb62eee3685fc4c43992febcd9e75443";
    uint64_t correlationId = 1;

    // --- transfers (rest_transfers.cpp values) ---
    hyperliquid::UsdClassTransferRequest classTransferReq;
    classTransferReq.amount = 1;
    classTransferReq.toPerp = false;
    ws.usdClassTransfer(classTransferReq, correlationId++);
    betweenCallsDelay();

    hyperliquid::UsdSendRequest usdSendReq;
    usdSendReq.destination = destination;
    usdSendReq.amount = 1;
    ws.usdSend(usdSendReq, correlationId++);
    betweenCallsDelay();

    hyperliquid::SpotSendRequest spotSendReq;
    spotSendReq.destination = destination;
    spotSendReq.token = tokenId;
    spotSendReq.amount = 1;
    ws.spotSend(spotSendReq, correlationId++);
    betweenCallsDelay();

    hyperliquid::SendAssetRequest sendAssetReq;
    sendAssetReq.destination = destination;
    sendAssetReq.sourceDex = "";
    sendAssetReq.destinationDex = "";
    sendAssetReq.token = tokenId;
    sendAssetReq.amount = 1;
    sendAssetReq.fromSubAccount = "";
    ws.sendAsset(sendAssetReq, correlationId++);
    betweenCallsDelay();

    hyperliquid::Withdraw3Request withdrawReq;
    withdrawReq.destination = destination;
    withdrawReq.amount = 2; // withdrawals below ~$2 are typically rejected by the bridge
    ws.withdraw3(withdrawReq, correlationId++);
    betweenCallsDelay();

    // --- misc actions (rest_misc_actions.cpp values) ---
    ws.noop(correlationId++);
    betweenCallsDelay();

    hyperliquid::ReserveRequestWeightRequest reserveReq;
    reserveReq.weight = 10;
    ws.reserveRequestWeight(reserveReq, correlationId++);
    betweenCallsDelay();

    hyperliquid::AgentSendAssetRequest agentSendReq;
    agentSendReq.destination = wallet.accountAddress; // agentSendAsset requires the account's own address
    agentSendReq.sourceDex = "";
    agentSendReq.destinationDex = "";
    agentSendReq.token = tokenId;
    agentSendReq.amount = "1";
    ws.agentSendAsset(agentSendReq, correlationId++);
    betweenCallsDelay();

    hyperliquid::SendToEvmWithDataRequest evmReq;
    evmReq.token = tokenId;
    evmReq.amount = "1";
    evmReq.sourceDex = "";
    evmReq.destinationRecipient = destination;
    evmReq.addressEncoding = hyperliquid::AddressEncoding::Hex;
    evmReq.destinationChainId = 421614; // Arbitrum Sepolia testnet
    evmReq.gasLimit = 200000;
    evmReq.data = "0x";
    ws.sendToEvmWithData(evmReq, correlationId++);
    betweenCallsDelay();

    // --- staking (rest_staking.cpp values) ---
    ws.cDeposit(3ULL * 100000000ULL, correlationId++); // 3 HYPE
    betweenCallsDelay();

    hyperliquid::TokenDelegateRequest delegateReq;
    delegateReq.validator = "0x0000472d488d33b7329ca53bfcc3918961d55f8e"; // "Puffer Node"
    delegateReq.wei = 2ULL * 100000000ULL; // 2 HYPE
    delegateReq.isUndelegate = false;
    ws.tokenDelegate(delegateReq, correlationId++);
    betweenCallsDelay();

    // Delegations have a lock-up before they can be undelegated, and cWithdraw has a separate
    // multi-day unstaking queue - expect a lock-up/queue rejection here, same as rest_staking.cpp.
    ws.cWithdraw(1ULL * 100000000ULL, correlationId++);
    betweenCallsDelay();

    // --- agent / builder fee approvals (rest_approve_agent.cpp / rest_approve_builder_fee.cpp values) ---
    hyperliquid::ApproveAgentRequest approveReq;
    approveReq.agentAddress = destination;
    approveReq.agentName = "ws-backfill-example";
    ws.approveAgent(approveReq, correlationId++);
    betweenCallsDelay();

    hyperliquid::ApproveBuilderFeeRequest builderFeeReq;
    builderFeeReq.builder = destination;
    builderFeeReq.maxFeeRate = "0.001%";
    ws.approveBuilderFee(builderFeeReq, correlationId++);
    betweenCallsDelay();

    // Confirmed live (see rest_agent_set_abstraction.cpp) to consistently reject with "Abstraction
    // transition not allowed" even signed by a freshly-approved agent - expected exchange
    // behavior, not an SDK bug. Sent here from the main wallet's own connection (not a
    // separately-signed agent connection) purely to confirm the request reaches the exchange with
    // the right shape.
    ws.agentSetAbstraction(hyperliquid::UserAbstractionMode::UnifiedAccount, correlationId++);
    betweenCallsDelay();

    // --- abstraction toggles (rest_user_set_abstraction.cpp / rest_user_dex_abstraction.cpp values) ---
    hyperliquid::UserSetAbstractionRequest userSetReq;
    userSetReq.user = wallet.accountAddress;
    userSetReq.abstraction = hyperliquid::AbstractionMode::Disabled;
    ws.userSetAbstraction(userSetReq, correlationId++);
    betweenCallsDelay();

    hyperliquid::UserDexAbstractionRequest userDexReq;
    userDexReq.user = wallet.accountAddress;
    userDexReq.enabled = false; // enabled=true is confirmed to reject once already in UnifiedAccount mode
    ws.userDexAbstraction(userDexReq, correlationId++);
    betweenCallsDelay();

    // --- vault / HIP-3 (rest_vault_transfer.cpp / rest_hip3_liquidator_transfer.cpp values) ---
    hyperliquid::VaultTransferRequest vaultReq;
    vaultReq.vaultAddress = "0xa15099a30bbf2e68942d6f4c43d70d04faeab0a0"; // Hyperliquidity Provider (HLP)
    vaultReq.isDeposit = true;
    vaultReq.usd = 6.0; // vault minimum deposit is $5
    ws.vaultTransfer(vaultReq, correlationId++);
    betweenCallsDelay();

    // Only the HIP-3 DEX's designated backstop liquidator address may deposit/withdraw here - this
    // wallet isn't one, so a real, informative rejection is expected, same as
    // rest_hip3_liquidator_transfer.cpp. ntl must be a multiple of 1000 quote tokens (1e-6 units).
    hyperliquid::Hip3LiquidatorTransferRequest hip3Req;
    hip3Req.dex = "test";
    hip3Req.ntl = 1'000'000'000ULL;
    hip3Req.isDeposit = true;
    ws.hip3LiquidatorTransfer(hip3Req, correlationId++);
    betweenCallsDelay();

    // --- leverage / isolated margin (rest_leverage.cpp values) ---
    hyperliquid::UpdateLeverageRequest leverageReq;
    leverageReq.asset = "ETH";
    leverageReq.isCross = false;
    leverageReq.leverage = 10;
    ws.updateLeverage(leverageReq, correlationId++);
    betweenCallsDelay();

    // updateIsolatedMargin needs an existing isolated position to act on - rest_leverage.cpp opens
    // one first via placeOrder. This example doesn't, so a rejection here is expected; it still
    // confirms the request reaches the correct business-logic check.
    hyperliquid::UpdateIsolatedMarginRequest marginReq;
    marginReq.asset = "ETH";
    marginReq.isBuy = true;
    marginReq.ntli = 1000000; // $1.00 (6 decimals)
    ws.updateIsolatedMargin(marginReq, correlationId++);
    betweenCallsDelay();

    // --- twap (rest_twap.cpp values) ---
    hyperliquid::TwapOrderRequest twapReq;
    twapReq.asset = "ETH";
    twapReq.isBuy = true;
    twapReq.size = 0.05;
    twapReq.reduceOnly = false;
    twapReq.minutes = 10;
    twapReq.randomize = true;
    ws.twapOrder(twapReq, correlationId++);
    betweenCallsDelay();

    // Real twapId chaining would require parsing the twapOrder response above (see ws_orders.cpp
    // for that RestApiMessageParser pattern) - out of scope for this wiring smoke test, so this
    // uses a placeholder id and expects a "twap not found"-style rejection rather than a real
    // cancel.
    hyperliquid::TwapCancelRequest cancelReq;
    cancelReq.asset = "ETH";
    cancelReq.twapId = 0;
    ws.twapCancel(cancelReq, correlationId++);
    betweenCallsDelay();

    spdlog::info("Done, stopping websocket...");
    ws.stop();

    return 0;
}
