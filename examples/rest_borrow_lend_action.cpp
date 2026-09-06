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

    // Supply a small, reversible amount of token 0 (USDC) to the borrow/lend pool. amount =
    // std::nullopt would mean "full amount" instead of a specific decimal quantity.
    hyperliquid::BorrowLendRequest supplyReq;
    supplyReq.operation = hyperliquid::BorrowLendOperation::Supply;
    supplyReq.token = 0;
    supplyReq.amount = 1.0;

    auto supplyResp = api.borrowLend(supplyReq);
    spdlog::info("borrowLend(supply): status={} type={}", supplyResp.status, supplyResp.type);
    if (supplyResp.error)
        spdlog::info("  Error: {}", *supplyResp.error);

    // TODO: coordinator will add live testnet evidence here (supply then withdraw a small
    // reversible amount)

    return 0;
}
