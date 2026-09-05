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

    hyperliquid::VaultTransferRequest depositReq;
    depositReq.vaultAddress = "0xa15099a30bbf2e68942d6f4c43d70d04faeab0a0"; // Hyperliquidity Provider (HLP)
    depositReq.isDeposit = true;
    depositReq.usd = 6.0; // vault minimum deposit is $5

    auto depositResp = api.vaultTransfer(depositReq);
    spdlog::info("vaultTransfer (deposit): status={} type={}", depositResp.status, depositResp.type);
    if (depositResp.error)
        spdlog::info("  Error: {}", *depositResp.error);

    return 0;
}
