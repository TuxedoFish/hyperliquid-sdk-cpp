#include <gtest/gtest.h>

#include "hyperliquid/websocket/WebsocketApi.h"
#include "hyperliquid/websocket/WebsocketApiListener.h"
#include "hyperliquid/config/Config.h"

using namespace hyperliquid;

namespace
{
    ApiConfig makeConfig()
    {
        ApiConfig config;
        config.env = Environment::Testnet;
        return config;
    }
}

TEST(WebsocketApiExchangeBackfill, TransferActionsThrowWithoutWallet)
{
    WebsocketApiListener listener;
    auto config = makeConfig();
    WebsocketApi ws(config, listener);

    UsdClassTransferRequest classTransferReq;
    classTransferReq.amount = 1;
    classTransferReq.toPerp = false;
    EXPECT_THROW(ws.usdClassTransfer(classTransferReq, 1), std::invalid_argument);

    UsdSendRequest usdSendReq;
    usdSendReq.destination = "0x5e9ee1089755c3435139848e47e6635505d5a13a";
    usdSendReq.amount = 1;
    EXPECT_THROW(ws.usdSend(usdSendReq, 2), std::invalid_argument);

    SpotSendRequest spotSendReq;
    spotSendReq.destination = "0x5e9ee1089755c3435139848e47e6635505d5a13a";
    spotSendReq.token = "USDC:0xeb62eee3685fc4c43992febcd9e75443";
    spotSendReq.amount = 1;
    EXPECT_THROW(ws.spotSend(spotSendReq, 3), std::invalid_argument);

    SendAssetRequest sendAssetReq;
    sendAssetReq.destination = "0x5e9ee1089755c3435139848e47e6635505d5a13a";
    sendAssetReq.sourceDex = "";
    sendAssetReq.destinationDex = "";
    sendAssetReq.token = "USDC:0xeb62eee3685fc4c43992febcd9e75443";
    sendAssetReq.amount = 1;
    sendAssetReq.fromSubAccount = "";
    EXPECT_THROW(ws.sendAsset(sendAssetReq, 4), std::invalid_argument);

    Withdraw3Request withdrawReq;
    withdrawReq.destination = "0x5e9ee1089755c3435139848e47e6635505d5a13a";
    withdrawReq.amount = 2;
    EXPECT_THROW(ws.withdraw3(withdrawReq, 5), std::invalid_argument);

    AgentSendAssetRequest agentSendReq;
    agentSendReq.destination = "0x5e9ee1089755c3435139848e47e6635505d5a13a";
    agentSendReq.sourceDex = "";
    agentSendReq.destinationDex = "";
    agentSendReq.token = "USDC:0xeb62eee3685fc4c43992febcd9e75443";
    agentSendReq.amount = "1";
    EXPECT_THROW(ws.agentSendAsset(agentSendReq, 6), std::invalid_argument);
    // vaultAddress is already a parameter on RestApi::agentSendAsset - confirm the ws wrapper
    // reuses that same trailing slot rather than duplicating it.
    EXPECT_THROW(ws.agentSendAsset(agentSendReq, 7, "0x1111111111111111111111111111111111111a"), std::invalid_argument);

    SendToEvmWithDataRequest evmReq;
    evmReq.token = "USDC:0xeb62eee3685fc4c43992febcd9e75443";
    evmReq.amount = "1";
    evmReq.sourceDex = "";
    evmReq.destinationRecipient = "0x5e9ee1089755c3435139848e47e6635505d5a13a";
    evmReq.addressEncoding = AddressEncoding::Hex;
    evmReq.destinationChainId = 421614;
    evmReq.gasLimit = 200000;
    evmReq.data = "0x";
    EXPECT_THROW(ws.sendToEvmWithData(evmReq, 8), std::invalid_argument);
}

TEST(WebsocketApiExchangeBackfill, VaultAndHip3ActionsThrowWithoutWallet)
{
    WebsocketApiListener listener;
    auto config = makeConfig();
    WebsocketApi ws(config, listener);

    VaultTransferRequest vaultReq;
    vaultReq.vaultAddress = "0xa15099a30bbf2e68942d6f4c43d70d04faeab0a0";
    vaultReq.isDeposit = true;
    vaultReq.usd = 6.0;
    EXPECT_THROW(ws.vaultTransfer(vaultReq, 1), std::invalid_argument);

    Hip3LiquidatorTransferRequest hip3Req;
    hip3Req.dex = "test";
    hip3Req.ntl = 1'000'000'000ULL;
    hip3Req.isDeposit = true;
    EXPECT_THROW(ws.hip3LiquidatorTransfer(hip3Req, 2), std::invalid_argument);

    CreateSubAccountRequest createSubAccountReq;
    createSubAccountReq.name = "example";
    EXPECT_THROW(ws.createSubAccount(createSubAccountReq, 3), std::invalid_argument);

    SubAccountTransferRequest subAccountTransferReq;
    subAccountTransferReq.subAccountUser = "0x1d9470d4b963f552e6f671a81619d395877bf409";
    subAccountTransferReq.isDeposit = true;
    subAccountTransferReq.usd = 10.0;
    EXPECT_THROW(ws.subAccountTransfer(subAccountTransferReq, 4), std::invalid_argument);
}

TEST(WebsocketApiExchangeBackfill, StakingActionsThrowWithoutWallet)
{
    WebsocketApiListener listener;
    auto config = makeConfig();
    WebsocketApi ws(config, listener);

    EXPECT_THROW(ws.cDeposit(3ULL * 100000000ULL, 1), std::invalid_argument);
    EXPECT_THROW(ws.cWithdraw(1ULL * 100000000ULL, 2), std::invalid_argument);

    TokenDelegateRequest delegateReq;
    delegateReq.validator = "0x0000472d488d33b7329ca53bfcc3918961d55f8e";
    delegateReq.wei = 2ULL * 100000000ULL;
    delegateReq.isUndelegate = false;
    EXPECT_THROW(ws.tokenDelegate(delegateReq, 3), std::invalid_argument);
}

TEST(WebsocketApiExchangeBackfill, LeverageMarginAndTwapActionsThrowWithoutWallet)
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
    EXPECT_THROW(ws.updateLeverage(leverageReq, 1), std::invalid_argument);

    UpdateIsolatedMarginRequest marginReq;
    marginReq.asset = "ETH";
    marginReq.assetId = 4;
    marginReq.isBuy = true;
    marginReq.ntli = 1000000;
    EXPECT_THROW(ws.updateIsolatedMargin(marginReq, 2), std::invalid_argument);

    TwapOrderRequest twapReq;
    twapReq.asset = "ETH";
    twapReq.assetId = 4;
    twapReq.isBuy = true;
    twapReq.size = 0.05;
    twapReq.reduceOnly = false;
    twapReq.minutes = 10;
    twapReq.randomize = true;
    EXPECT_THROW(ws.twapOrder(twapReq, 3), std::invalid_argument);

    TwapCancelRequest cancelReq;
    cancelReq.asset = "ETH";
    cancelReq.assetId = 4;
    cancelReq.twapId = 12345;
    EXPECT_THROW(ws.twapCancel(cancelReq, 4), std::invalid_argument);
}

TEST(WebsocketApiExchangeBackfill, AgentAndAbstractionActionsThrowWithoutWallet)
{
    WebsocketApiListener listener;
    auto config = makeConfig();
    WebsocketApi ws(config, listener);

    ApproveAgentRequest approveReq;
    approveReq.agentAddress = "0x5e9ee1089755c3435139848e47e6635505d5a13a";
    approveReq.agentName = "backfill-test";
    EXPECT_THROW(ws.approveAgent(approveReq, 1), std::invalid_argument);

    ApproveBuilderFeeRequest builderFeeReq;
    builderFeeReq.builder = "0x5e9ee1089755c3435139848e47e6635505d5a13a";
    builderFeeReq.maxFeeRate = "0.001%";
    EXPECT_THROW(ws.approveBuilderFee(builderFeeReq, 2), std::invalid_argument);

    // agentSetAbstraction already takes vaultAddress on RestApi - confirm the ws wrapper reuses
    // that slot rather than duplicating it.
    EXPECT_THROW(ws.agentSetAbstraction(UserAbstractionMode::UnifiedAccount, 3), std::invalid_argument);

    UserSetAbstractionRequest userSetReq;
    userSetReq.user = "0x5e9ee1089755c3435139848e47e6635505d5a13a";
    userSetReq.abstraction = AbstractionMode::Disabled;
    EXPECT_THROW(ws.userSetAbstraction(userSetReq, 4), std::invalid_argument);

    UserDexAbstractionRequest userDexReq;
    userDexReq.user = "0x5e9ee1089755c3435139848e47e6635505d5a13a";
    userDexReq.enabled = false;
    EXPECT_THROW(ws.userDexAbstraction(userDexReq, 5), std::invalid_argument);
}

TEST(WebsocketApiExchangeBackfill, MiscActionsThrowWithoutWallet)
{
    WebsocketApiListener listener;
    auto config = makeConfig();
    WebsocketApi ws(config, listener);

    // noop/reserveRequestWeight already take vaultAddress on RestApi - confirm the ws wrapper
    // reuses that same trailing slot rather than duplicating it.
    EXPECT_THROW(ws.noop(1), std::invalid_argument);
    EXPECT_THROW(ws.noop(2, "0x1111111111111111111111111111111111111a"), std::invalid_argument);

    ReserveRequestWeightRequest reserveReq;
    reserveReq.weight = 10;
    EXPECT_THROW(ws.reserveRequestWeight(reserveReq, 3), std::invalid_argument);
}
