// Live smoke test for the 43 WebsocketApi info-endpoint wrappers backfilled in this PR.
//
// Each call below wraps an InfoRequestBuilder-generated payload in a websocket "post" envelope
// via WebsocketApi::Impl::signAndSend and expects a correlated response back on the "post"
// channel. This example does not assert anything - it just logs the raw JSON for each response
// (tagged with its RestEndpointType and correlationId) so a human can eyeball real server
// responses and catch send-side bugs (wrong param shape, wrong RestEndpointType mapping, a
// request the server rejects outright) that a unit test against InfoRequestBuilder alone cannot
// catch, per the same "run it live" philosophy CONTRIBUTING.md documents for REST endpoints.

#include <chrono>
#include <thread>

#include <spdlog/spdlog.h>

#include "test_config.h"
#include "hyperliquid/types/RequestTypes.h"
#include "hyperliquid/websocket/WebsocketApi.h"
#include "hyperliquid/websocket/WebsocketApiListener.h"

class PostResponseLogger : public hyperliquid::WebsocketApiListener
{
public:
    void onPostResponse(const std::string& rawJson, hyperliquid::RestEndpointType type,
                        std::optional<uint64_t> correlationId) override
    {
        spdlog::info("[post response] type={} correlationId={} payload={}",
                     hyperliquid::toString(type), correlationId.value_or(0), rawJson);
    }

    void onConnected() override
    {
        spdlog::info("Connected");
    }

    void onDisconnected(bool hasError, const std::string& errMsg) override
    {
        spdlog::info("Disconnected: hasError={} errMsg={}", hasError, errMsg);
    }
};

int main()
{
    // Only the account address is used below - the private key is never read or logged. All
    // requests here are plain unauthenticated /info reads, so no wallet is required, but a real
    // address gives more meaningful responses for user-scoped endpoints.
    std::string userAddress = loadWalletFromConfig().accountAddress;

    hyperliquid::setLogLevel(hyperliquid::LogLevel::Info);

    hyperliquid::ApiConfig config;
    config.env = hyperliquid::Environment::Testnet;
    config.skipBuildingSymbolMap = true;

    PostResponseLogger logger;
    hyperliquid::WebsocketApi ws(config, logger);

    spdlog::info("Starting websocket...");
    ws.start();
    std::this_thread::sleep_for(std::chrono::seconds(2));

    const uint64_t now = static_cast<uint64_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count());
    const uint64_t oneHourAgo = now - 3600ULL * 1000ULL;

    // Well-known testnet builder/vault addresses used elsewhere in this repo's examples.
    const std::string builderAddress = "0x1719884eb866cb12b2287399b15f7db5e7d775ea";
    const std::string vaultAddress = "0x1719884eb866cb12b2287399b15f7db5e7d775ea";
    // Real HIP-3 testnet dex name, used elsewhere in this repo's examples (rest_hip3_deployer.cpp).
    const std::string dex = "hyna";

    uint64_t correlationId = 1;
    auto wait = []() { std::this_thread::sleep_for(std::chrono::milliseconds(300)); };

    ws.settledOutcome(1000, correlationId++); wait();
    ws.perpsAtOpenInterestCap(std::nullopt, correlationId++); wait();
    ws.predictedFundings(correlationId++); wait();
    ws.perpAnnotation("BTC", correlationId++); wait();
    ws.perpCategories(correlationId++); wait();
    ws.perpConciseAnnotations(correlationId++); wait();
    ws.allPerpMetas(correlationId++); wait();
    ws.perpDexLimits(dex, correlationId++); wait();
    ws.perpDexStatus(dex, correlationId++); wait();
    ws.perpDeployAuctionStatus(correlationId++); wait();

    ws.l2Book("BTC", std::nullopt, std::nullopt, correlationId++); wait();
    ws.candleSnapshot("BTC", "15m", oneHourAgo, now, correlationId++); wait();
    ws.allMids(std::nullopt, correlationId++); wait();
    ws.openOrders(userAddress, std::nullopt, correlationId++); wait();
    // Plausible-but-unlikely-to-exist oid: exercises the request/response wiring even though
    // the server is expected to reply with a "not found"-style response rather than an order.
    ws.orderStatus(userAddress, hyperliquid::OrderId{12345678ULL}, correlationId++); wait();
    ws.userFills(userAddress, std::nullopt, std::nullopt, correlationId++); wait();
    ws.userFillsByTime(userAddress, oneHourAgo, std::nullopt, std::nullopt, std::nullopt, correlationId++); wait();
    ws.clearinghouseState(userAddress, std::nullopt, correlationId++); wait();
    ws.userRateLimit(userAddress, correlationId++); wait();
    ws.metaAndAssetCtxs(std::nullopt, correlationId++); wait();
    ws.spotMetaAndAssetCtxs(correlationId++); wait();
    ws.spotClearinghouseState(userAddress, std::nullopt, correlationId++); wait();
    ws.spotDeployState(userAddress, correlationId++); wait();
    ws.spotPairDeployAuctionStatus(correlationId++); wait();
    ws.frontendOpenOrders(userAddress, std::nullopt, correlationId++); wait();
    ws.historicalOrders(userAddress, correlationId++); wait();
    ws.userTwapSliceFills(userAddress, correlationId++); wait();
    ws.subAccounts(userAddress, correlationId++); wait();
    ws.userFees(userAddress, correlationId++); wait();
    ws.maxBuilderFee(userAddress, builderAddress, correlationId++); wait();
    ws.approvedBuilders(userAddress, correlationId++); wait();

    ws.delegations(userAddress, correlationId++); wait();
    ws.delegatorSummary(userAddress, correlationId++); wait();
    ws.delegatorHistory(userAddress, correlationId++); wait();
    ws.delegatorRewards(userAddress, correlationId++); wait();

    ws.vaultDetails(vaultAddress, std::nullopt, correlationId++); wait();
    ws.userVaultEquities(userAddress, correlationId++); wait();
    ws.portfolio(userAddress, correlationId++); wait();
    ws.referral(userAddress, correlationId++); wait();
    ws.userRole(userAddress, correlationId++); wait();

    ws.borrowLendUserState(userAddress, correlationId++); wait();
    ws.borrowLendReserveState(0, correlationId++); wait();
    ws.allBorrowLendReserveStates(correlationId++); wait();

    spdlog::info("Sent {} post requests, waiting for stragglers...", correlationId - 1);
    std::this_thread::sleep_for(std::chrono::seconds(3));

    ws.stop();
    return 0;
}
