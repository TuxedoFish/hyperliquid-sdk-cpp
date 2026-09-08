#include "test_config.h"

#include <hyperliquid/rest/RestApi.h>
#include <hyperliquid/config/Config.h>
#include <spdlog/spdlog.h>

int main()
{
    auto wallet = loadWalletFromConfig();
    hyperliquid::setLogLevel(hyperliquid::LogLevel::Debug);

    hyperliquid::ApiConfig config;
    config.env = hyperliquid::Environment::Testnet;
    config.wallet = wallet;

    hyperliquid::RestApi api(config);

    hyperliquid::CreateSubAccountRequest createReq;
    createReq.name = "example-sub-account";

    auto createResp = api.createSubAccount(createReq);
    spdlog::info("createSubAccount: status={} type={}", createResp.status, createResp.type);
    if (createResp.error)
        spdlog::info("  Error: {}", *createResp.error);
    if (!createResp.subAccountUser)
    {
        spdlog::error("createSubAccount did not return a sub-account address, aborting");
        return 1;
    }
    spdlog::info("  New sub-account address: {}", *createResp.subAccountUser);

    hyperliquid::SubAccountTransferRequest transferReq;
    transferReq.subAccountUser = *createResp.subAccountUser;
    transferReq.isDeposit = true;
    transferReq.usd = 1.0;

    auto transferResp = api.subAccountTransfer(transferReq);
    spdlog::info("subAccountTransfer (deposit): status={} type={}", transferResp.status, transferResp.type);
    if (transferResp.error)
        spdlog::info("  Error: {}", *transferResp.error);

    return 0;
}
