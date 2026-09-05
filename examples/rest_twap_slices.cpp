#include "test_config.h"

#include <hyperliquid/rest/RestApi.h>
#include <hyperliquid/config/Config.h>
#include <spdlog/spdlog.h>

#include <chrono>
#include <thread>

int main()
{
    auto wallet = loadWalletFromConfig();
    std::string userAddress = wallet.accountAddress;
    hyperliquid::setLogLevel(hyperliquid::LogLevel::Info);

    hyperliquid::ApiConfig config;
    config.env = hyperliquid::Environment::Testnet;
    config.wallet = wallet;

    hyperliquid::RestApi api(config);

    auto before = api.clearinghouseState(userAddress);
    spdlog::info("perp accountValue: {}", before.marginSummary.accountValue);

    spdlog::info("=== twapOrder: buy 0.0013 BTC (~$105 notional, min is $100) over 5 minutes ===");
    hyperliquid::TwapOrderRequest twapReq;
    twapReq.asset = "BTC";
    twapReq.isBuy = true;
    twapReq.size = 0.0013;
    twapReq.reduceOnly = false;
    twapReq.minutes = 5;
    twapReq.randomize = true;

    auto twapResp = api.twapOrder(twapReq);
    spdlog::info("twapOrder: status={} type={}", twapResp.status, twapResp.type);
    if (twapResp.error)
        spdlog::info("  Error: {}", *twapResp.error);
    if (!twapResp.twapId)
    {
        spdlog::error("No twapId returned, aborting slice-fill polling.");
        return 1;
    }
    spdlog::info("  Running twapId={}", *twapResp.twapId);

    for (int i = 0; i < 5; ++i)
    {
        std::this_thread::sleep_for(std::chrono::seconds(15));
        auto slices = api.userTwapSliceFills(userAddress);
        spdlog::info("[poll {}] userTwapSliceFills: {} total fills", i + 1, slices.fills.size());
        for (const auto& slice : slices.fills)
        {
            if (slice.twapId != *twapResp.twapId) continue;
            spdlog::info("  MATCH twapId={} coin={} px={} sz={} time={}",
                         slice.twapId, slice.fill.coin, slice.fill.px, slice.fill.sz, slice.fill.time);
        }
    }

    spdlog::info("=== twapCancel ===");
    hyperliquid::TwapCancelRequest cancelReq;
    cancelReq.asset = "BTC";
    cancelReq.twapId = *twapResp.twapId;
    auto cancelResp = api.twapCancel(cancelReq);
    spdlog::info("twapCancel: status={} type={}", cancelResp.status, cancelResp.type);
    if (cancelResp.success)
        spdlog::info("  {}", *cancelResp.success);
    if (cancelResp.error)
        spdlog::info("  Error: {}", *cancelResp.error);

    return 0;
}
