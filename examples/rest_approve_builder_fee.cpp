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

    hyperliquid::ApproveBuilderFeeRequest req;
    req.builder = "0x5e9ee1089755c3435139848e47e6635505d5a13a";
    req.maxFeeRate = "0.001%";

    auto resp = api.approveBuilderFee(req);
    spdlog::info("approveBuilderFee: status={} type={}", resp.status, resp.type);
    if (resp.error)
        spdlog::info("  Error: {}", *resp.error);

    return 0;
}
