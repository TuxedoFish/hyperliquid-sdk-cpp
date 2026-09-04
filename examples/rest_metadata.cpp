#include "test_config.h"

#include <hyperliquid/rest/RestApi.h>
#include <hyperliquid/config/Config.h>
#include <spdlog/spdlog.h>

int main()
{
    // Only the account address is used below - the private key is never read or logged.
    std::string userAddress = loadWalletFromConfig().accountAddress;

    hyperliquid::setLogLevel(hyperliquid::LogLevel::Info);

    hyperliquid::ApiConfig config;
    config.env = hyperliquid::Environment::Testnet;
    config.skipBuildingSymbolMap = true;

    hyperliquid::RestApi api(config);

    spdlog::info("=== metaAndAssetCtxs ===");
    auto metaCtxs = api.metaAndAssetCtxs();
    spdlog::info("universe={} assetCtxs={}", metaCtxs.meta.universe.size(), metaCtxs.assetCtxs.size());
    if (!metaCtxs.assetCtxs.empty())
        spdlog::info("  first: markPx={} funding={} openInterest={}",
                     metaCtxs.assetCtxs[0].markPx, metaCtxs.assetCtxs[0].funding, metaCtxs.assetCtxs[0].openInterest);

    spdlog::info("=== spotMetaAndAssetCtxs ===");
    auto spotMetaCtxs = api.spotMetaAndAssetCtxs();
    spdlog::info("tokens={} universe={} assetCtxs={}",
                 spotMetaCtxs.meta.tokens.size(), spotMetaCtxs.meta.universe.size(), spotMetaCtxs.assetCtxs.size());

    if (!userAddress.empty())
    {
        spdlog::info("=== spotClearinghouseState ===");
        auto spotState = api.spotClearinghouseState(userAddress);
        spdlog::info("{} balances", spotState.balances.size());
        for (const auto& balance : spotState.balances)
            spdlog::info("  coin={} total={} hold={}", balance.coin, balance.total, balance.hold);

        spdlog::info("=== frontendOpenOrders ===");
        auto frontendOrders = api.frontendOpenOrders(userAddress);
        spdlog::info("{} orders", frontendOrders.orders.size());
        for (const auto& order : frontendOrders.orders)
            spdlog::info("  coin={} side={} limitPx={} sz={} orderType={}",
                         order.coin, order.side, order.limitPx, order.sz, hyperliquid::toString(order.orderType));

        spdlog::info("=== historicalOrders ===");
        auto historicalOrders = api.historicalOrders(userAddress);
        spdlog::info("{} orders", historicalOrders.orders.size());
        for (size_t i = 0; i < historicalOrders.orders.size() && i < 3; ++i)
            spdlog::info("  coin={} status={} statusTimestamp={}",
                         historicalOrders.orders[i].order.coin,
                         hyperliquid::toString(historicalOrders.orders[i].status),
                         historicalOrders.orders[i].statusTimestamp);

        spdlog::info("=== userTwapSliceFills ===");
        auto twapFills = api.userTwapSliceFills(userAddress);
        spdlog::info("{} twap slice fills", twapFills.fills.size());

        spdlog::info("=== subAccounts ===");
        auto subAccounts = api.subAccounts(userAddress);
        spdlog::info("{} sub-accounts", subAccounts.subAccounts.size());
        for (const auto& sub : subAccounts.subAccounts)
            spdlog::info("  name={} subAccountUser={}", sub.name, sub.subAccountUser);

        spdlog::info("=== userFees ===");
        auto fees = api.userFees(userAddress);
        spdlog::info("userCrossRate={} userAddRate={} dailyUserVlmDays={}",
                     fees.userCrossRate, fees.userAddRate, fees.dailyUserVlm.size());

        spdlog::info("=== maxBuilderFee ===");
        // Well-known testnet builder address used elsewhere in this repo's tests/examples.
        std::string builderAddress = "0x1719884eb866cb12b2287399b15f7db5e7d775ea";
        auto maxFee = api.maxBuilderFee(userAddress, builderAddress);
        spdlog::info("maxFeeRateTenthsBps={}", maxFee.maxFeeRateTenthsBps);

        spdlog::info("=== approvedBuilders ===");
        auto approvedBuilders = api.approvedBuilders(userAddress);
        spdlog::info("{} approved builders", approvedBuilders.builders.size());
        for (const auto& builder : approvedBuilders.builders)
            spdlog::info("  {}", builder);
    }

    return 0;
}
