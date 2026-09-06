#include <gtest/gtest.h>

#include "hyperliquid/websocket/WebsocketApi.h"
#include "hyperliquid/websocket/WebsocketApiListener.h"
#include "hyperliquid/config/Config.h"

using namespace hyperliquid;

namespace
{
    // No wallet is configured, so WebsocketApi's constructor skips the network call it would
    // otherwise make to build its symbol map, and Signing::prepareBody logs-and-returns instead
    // of throwing when it finds no wallet to sign with. start()/stop() are never called either,
    // so WebsocketRunner::send() just enqueues onto an io_context that's never run. This exercises
    // the real WebsocketApi::x -> ExchangeRequestBuilder::x -> signAndSend wiring for each of the
    // newly-backfilled exchange action wrappers without touching the network - a bug in the wiring
    // (wrong builder call, wrong RestEndpointType, wrong parameter forwarding) would either fail to
    // compile or throw here.
    ApiConfig makeConfig()
    {
        ApiConfig config;
        config.env = Environment::Testnet;
        return config;
    }
}

TEST(WebsocketApiExchangeBackfill, TransferActionsDoNotThrow)
{
    WebsocketApiListener listener;
    auto config = makeConfig();
    WebsocketApi ws(config, listener);

    UsdClassTransferRequest classTransferReq;
    classTransferReq.amount = 1;
    classTransferReq.toPerp = false;
    EXPECT_NO_THROW(ws.usdClassTransfer(classTransferReq, 1));

    UsdSendRequest usdSendReq;
    usdSendReq.destination = "0x5e9ee1089755c3435139848e47e6635505d5a13a";
    usdSendReq.amount = 1;
    EXPECT_NO_THROW(ws.usdSend(usdSendReq, 2));

    SpotSendRequest spotSendReq;
    spotSendReq.destination = "0x5e9ee1089755c3435139848e47e6635505d5a13a";
    spotSendReq.token = "USDC:0xeb62eee3685fc4c43992febcd9e75443";
    spotSendReq.amount = 1;
    EXPECT_NO_THROW(ws.spotSend(spotSendReq, 3));

    SendAssetRequest sendAssetReq;
    sendAssetReq.destination = "0x5e9ee1089755c3435139848e47e6635505d5a13a";
    sendAssetReq.sourceDex = "";
    sendAssetReq.destinationDex = "";
    sendAssetReq.token = "USDC:0xeb62eee3685fc4c43992febcd9e75443";
    sendAssetReq.amount = 1;
    sendAssetReq.fromSubAccount = "";
    EXPECT_NO_THROW(ws.sendAsset(sendAssetReq, 4));

    Withdraw3Request withdrawReq;
    withdrawReq.destination = "0x5e9ee1089755c3435139848e47e6635505d5a13a";
    withdrawReq.amount = 2;
    EXPECT_NO_THROW(ws.withdraw3(withdrawReq, 5));

    AgentSendAssetRequest agentSendReq;
    agentSendReq.destination = "0x5e9ee1089755c3435139848e47e6635505d5a13a";
    agentSendReq.sourceDex = "";
    agentSendReq.destinationDex = "";
    agentSendReq.token = "USDC:0xeb62eee3685fc4c43992febcd9e75443";
    agentSendReq.amount = "1";
    EXPECT_NO_THROW(ws.agentSendAsset(agentSendReq, 6));
    // vaultAddress is already a parameter on RestApi::agentSendAsset - confirm the ws wrapper
    // reuses that same trailing slot rather than duplicating it.
    EXPECT_NO_THROW(ws.agentSendAsset(agentSendReq, 7, "0x1111111111111111111111111111111111111a"));

    SendToEvmWithDataRequest evmReq;
    evmReq.token = "USDC:0xeb62eee3685fc4c43992febcd9e75443";
    evmReq.amount = "1";
    evmReq.sourceDex = "";
    evmReq.destinationRecipient = "0x5e9ee1089755c3435139848e47e6635505d5a13a";
    evmReq.addressEncoding = AddressEncoding::Hex;
    evmReq.destinationChainId = 421614;
    evmReq.gasLimit = 200000;
    evmReq.data = "0x";
    EXPECT_NO_THROW(ws.sendToEvmWithData(evmReq, 8));
}

TEST(WebsocketApiExchangeBackfill, VaultAndHip3ActionsDoNotThrow)
{
    WebsocketApiListener listener;
    auto config = makeConfig();
    WebsocketApi ws(config, listener);

    VaultTransferRequest vaultReq;
    vaultReq.vaultAddress = "0xa15099a30bbf2e68942d6f4c43d70d04faeab0a0";
    vaultReq.isDeposit = true;
    vaultReq.usd = 6.0;
    EXPECT_NO_THROW(ws.vaultTransfer(vaultReq, 1));

    Hip3LiquidatorTransferRequest hip3Req;
    hip3Req.dex = "test";
    hip3Req.ntl = 1'000'000'000ULL;
    hip3Req.isDeposit = true;
    EXPECT_NO_THROW(ws.hip3LiquidatorTransfer(hip3Req, 2));
}

TEST(WebsocketApiExchangeBackfill, StakingActionsDoNotThrow)
{
    WebsocketApiListener listener;
    auto config = makeConfig();
    WebsocketApi ws(config, listener);

    EXPECT_NO_THROW(ws.cDeposit(3ULL * 100000000ULL, 1));
    EXPECT_NO_THROW(ws.cWithdraw(1ULL * 100000000ULL, 2));

    TokenDelegateRequest delegateReq;
    delegateReq.validator = "0x0000472d488d33b7329ca53bfcc3918961d55f8e";
    delegateReq.wei = 2ULL * 100000000ULL;
    delegateReq.isUndelegate = false;
    EXPECT_NO_THROW(ws.tokenDelegate(delegateReq, 3));
}

TEST(WebsocketApiExchangeBackfill, LeverageMarginAndTwapActionsDoNotThrow)
{
    WebsocketApiListener listener;
    auto config = makeConfig();
    WebsocketApi ws(config, listener);

    // assetId is set explicitly so these don't need a populated symbol map (which would
    // otherwise require a real meta() network call - this config has no wallet, so
    // ExchangeRequestBuilder::initializeMapping's symbol map build never ran).
    UpdateLeverageRequest leverageReq;
    leverageReq.asset = "ETH";
    leverageReq.assetId = 4;
    leverageReq.isCross = false;
    leverageReq.leverage = 10;
    EXPECT_NO_THROW(ws.updateLeverage(leverageReq, 1));

    UpdateIsolatedMarginRequest marginReq;
    marginReq.asset = "ETH";
    marginReq.assetId = 4;
    marginReq.isBuy = true;
    marginReq.ntli = 1000000;
    EXPECT_NO_THROW(ws.updateIsolatedMargin(marginReq, 2));

    TwapOrderRequest twapReq;
    twapReq.asset = "ETH";
    twapReq.assetId = 4;
    twapReq.isBuy = true;
    twapReq.size = 0.05;
    twapReq.reduceOnly = false;
    twapReq.minutes = 10;
    twapReq.randomize = true;
    EXPECT_NO_THROW(ws.twapOrder(twapReq, 3));

    TwapCancelRequest cancelReq;
    cancelReq.asset = "ETH";
    cancelReq.assetId = 4;
    cancelReq.twapId = 12345;
    EXPECT_NO_THROW(ws.twapCancel(cancelReq, 4));
}

TEST(WebsocketApiExchangeBackfill, AgentAndAbstractionActionsDoNotThrow)
{
    WebsocketApiListener listener;
    auto config = makeConfig();
    WebsocketApi ws(config, listener);

    ApproveAgentRequest approveReq;
    approveReq.agentAddress = "0x5e9ee1089755c3435139848e47e6635505d5a13a";
    approveReq.agentName = "backfill-test";
    EXPECT_NO_THROW(ws.approveAgent(approveReq, 1));

    ApproveBuilderFeeRequest builderFeeReq;
    builderFeeReq.builder = "0x5e9ee1089755c3435139848e47e6635505d5a13a";
    builderFeeReq.maxFeeRate = "0.001%";
    EXPECT_NO_THROW(ws.approveBuilderFee(builderFeeReq, 2));

    // agentSetAbstraction already takes vaultAddress on RestApi - confirm the ws wrapper reuses
    // that slot rather than duplicating it.
    EXPECT_NO_THROW(ws.agentSetAbstraction(UserAbstractionMode::UnifiedAccount, 3));

    UserSetAbstractionRequest userSetReq;
    userSetReq.user = "0x5e9ee1089755c3435139848e47e6635505d5a13a";
    userSetReq.abstraction = AbstractionMode::Disabled;
    EXPECT_NO_THROW(ws.userSetAbstraction(userSetReq, 4));

    UserDexAbstractionRequest userDexReq;
    userDexReq.user = "0x5e9ee1089755c3435139848e47e6635505d5a13a";
    userDexReq.enabled = false;
    EXPECT_NO_THROW(ws.userDexAbstraction(userDexReq, 5));
}

TEST(WebsocketApiExchangeBackfill, MiscActionsDoNotThrow)
{
    WebsocketApiListener listener;
    auto config = makeConfig();
    WebsocketApi ws(config, listener);

    // noop/reserveRequestWeight already take vaultAddress on RestApi - confirm the ws wrapper
    // reuses that same trailing slot rather than duplicating it.
    EXPECT_NO_THROW(ws.noop(1));
    EXPECT_NO_THROW(ws.noop(2, "0x1111111111111111111111111111111111111a"));

    ReserveRequestWeightRequest reserveReq;
    reserveReq.weight = 10;
    EXPECT_NO_THROW(ws.reserveRequestWeight(reserveReq, 3));
}
