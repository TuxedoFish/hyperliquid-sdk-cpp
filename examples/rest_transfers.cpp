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

    hyperliquid::UsdClassTransferRequest classTransferReq;
    classTransferReq.amount = 1;
    classTransferReq.toPerp = false; // move $1 from perp to spot
    logSimpleResponse("usdClassTransfer", api.usdClassTransfer(classTransferReq));

    hyperliquid::UsdSendRequest usdSendReq;
    usdSendReq.destination = destination;
    usdSendReq.amount = 1;
    logSimpleResponse("usdSend", api.usdSend(usdSendReq));

    hyperliquid::SpotSendRequest spotSendReq;
    spotSendReq.destination = destination;
    spotSendReq.token = "USDC:0xeb62eee3685fc4c43992febcd9e75443"; // tokenName:tokenId, see spotMeta
    spotSendReq.amount = 1;
    logSimpleResponse("spotSend", api.spotSend(spotSendReq));

    hyperliquid::SendAssetRequest sendAssetReq;
    sendAssetReq.destination = destination;
    sendAssetReq.sourceDex = "";
    sendAssetReq.destinationDex = "";
    sendAssetReq.token = "USDC:0xeb62eee3685fc4c43992febcd9e75443";
    sendAssetReq.amount = 1;
    sendAssetReq.fromSubAccount = "";
    logSimpleResponse("sendAsset", api.sendAsset(sendAssetReq));

    hyperliquid::Withdraw3Request withdrawReq;
    withdrawReq.destination = destination;
    withdrawReq.amount = 2; // withdrawals below ~$2 are typically rejected by the bridge
    logSimpleResponse("withdraw3", api.withdraw3(withdrawReq));

    return 0;
}
