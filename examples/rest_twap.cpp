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

    hyperliquid::TwapOrderRequest twapReq;
    twapReq.asset = "ETH";
    twapReq.isBuy = true;
    twapReq.size = 0.05;
    twapReq.reduceOnly = false;
    twapReq.minutes = 10;
    twapReq.randomize = true;

    auto twapResp = api.twapOrder(twapReq);
    spdlog::info("twapOrder: status={} type={}", twapResp.status, twapResp.type);
    if (twapResp.twapId)
        spdlog::info("  Running twapId={}", *twapResp.twapId);
    if (twapResp.error)
        spdlog::info("  Error: {}", *twapResp.error);

    if (!twapResp.twapId)
        return 0;

    hyperliquid::TwapCancelRequest cancelReq;
    cancelReq.asset = "ETH";
    cancelReq.twapId = *twapResp.twapId;

    auto cancelResp = api.twapCancel(cancelReq);
    spdlog::info("twapCancel: status={} type={}", cancelResp.status, cancelResp.type);
    if (cancelResp.success)
        spdlog::info("  {}", *cancelResp.success);
    if (cancelResp.error)
        spdlog::info("  Error: {}", *cancelResp.error);

    return 0;
}
