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

    // Only the HIP-3 DEX's designated backstop liquidator address may deposit/withdraw here -
    // this wallet isn't one, so the exchange is expected to reject with a real, informative
    // error rather than silently succeeding. ntl must be a multiple of 1000 quote tokens
    // (1e-6 units), per the docs.
    hyperliquid::Hip3LiquidatorTransferRequest req;
    req.dex = "test";
    req.ntl = 1'000'000'000ULL;
    req.isDeposit = true;

    auto resp = api.hip3LiquidatorTransfer(req);
    spdlog::info("hip3LiquidatorTransfer: status={} type={}", resp.status, resp.type);
    if (resp.error)
        spdlog::info("  Error: {}", *resp.error);

    return 0;
}
