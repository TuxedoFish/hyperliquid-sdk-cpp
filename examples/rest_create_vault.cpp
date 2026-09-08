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

    hyperliquid::CreateVaultRequest req;
    req.name = "SDK Test Vault";
    req.description = "Vault created by hyperliquid-sdk-cpp's rest_create_vault example.";
    req.initialUsd = 100.0; // Hyperliquid requires at least 100 USD to create a vault.

    auto resp = api.createVault(req);
    spdlog::info("createVault: status={} type={}", resp.status, resp.type);
    if (resp.vaultAddress)
        spdlog::info("  New vault address: {}", *resp.vaultAddress);
    if (resp.error)
        spdlog::info("  Error: {}", *resp.error);

    return 0;
}
