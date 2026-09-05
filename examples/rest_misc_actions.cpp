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
    hyperliquid::setLogLevel(hyperliquid::LogLevel::Debug);

    hyperliquid::ApiConfig config;
    config.env = hyperliquid::Environment::Testnet;
    config.wallet = wallet;

    hyperliquid::RestApi api(config);

    const std::string destination = "0x5e9ee1089755c3435139848e47e6635505d5a13a";

    // noop is a real signed L1 action that does nothing to account state - used to keep a nonce
    // "fresh" or exercise the signing path without side effects.
    logSimpleResponse("noop", api.noop());

    // Reserves extra request-weight capacity ahead of time, paid for out of the account.
    hyperliquid::ReserveRequestWeightRequest reserveReq;
    reserveReq.weight = 10;
    logSimpleResponse("reserveRequestWeight", api.reserveRequestWeight(reserveReq));

    // Moves an asset between dexes (main perp dex <-> a HIP-3 builder-deployed dex) under the
    // calling account. Unlike sendAsset, this can be signed by an approved agent wallet - but
    // per the API docs, `destination` must be the account's own address (it moves funds between
    // that account's own dexes, not to a third party), so we use the wallet's own address here
    // rather than the third-party `destination` constant used above.
    // NOTE: at time of writing, testnet rejects this action with a generic
    // "Failed to deserialize the JSON body into the target type" regardless of field content
    // (verified against the documented request shape, with/without the optional fromSubAccount,
    // and with both checksummed and lowercase destination casing) - this looks like a testnet
    // rollout gap for this specific action type rather than a client-side request bug, since
    // sibling actions (sendAsset, sendToEvmWithData) with near-identical shapes get real,
    // properly-parsed business responses instead of a deserialize failure.
    hyperliquid::AgentSendAssetRequest agentSendReq;
    agentSendReq.destination = wallet.accountAddress;
    agentSendReq.sourceDex = "";
    agentSendReq.destinationDex = "";
    agentSendReq.token = "USDC:0xeb62eee3685fc4c43992febcd9e75443";
    agentSendReq.amount = "1";
    // fromSubAccount left unset - agentSendAsset's documented schema has no such field.
    logSimpleResponse("agentSendAsset", api.agentSendAsset(agentSendReq));

    // Bridges a token out to an arbitrary EVM chain/address with attached calldata. Niche
    // feature - destinationChainId/gasLimit below are illustrative (Arbitrum Sepolia testnet).
    hyperliquid::SendToEvmWithDataRequest evmReq;
    evmReq.token = "USDC:0xeb62eee3685fc4c43992febcd9e75443";
    evmReq.amount = "1";
    evmReq.sourceDex = "";
    evmReq.destinationRecipient = destination;
    evmReq.addressEncoding = hyperliquid::AddressEncoding::Hex;
    evmReq.destinationChainId = 421614;
    evmReq.gasLimit = 200000;
    evmReq.data = "0x";
    logSimpleResponse("sendToEvmWithData", api.sendToEvmWithData(evmReq));

    return 0;
}
