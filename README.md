# hyperliquid-sdk-cpp

[![CI](https://github.com/TuxedoFish/hyperliquid-sdk-cpp/actions/workflows/ci.yml/badge.svg)](https://github.com/TuxedoFish/hyperliquid-sdk-cpp/actions/workflows/ci.yml)
[![codecov](https://codecov.io/gh/TuxedoFish/hyperliquid-sdk-cpp/branch/main/graph/badge.svg)](https://codecov.io/gh/TuxedoFish/hyperliquid-sdk-cpp)

A C++23 SDK for the [Hyperliquid](https://hyperliquid.xyz) perpetuals and spot exchange API — REST (`/info`, `/exchange`) and WebSocket, with typed request/response models.

This SDK signs and submits real transactions on mainnet. Test against `Environment::Testnet` before pointing any code at `Environment::Mainnet`.

## Build

Requires CMake 3.16+, a C++23 compiler, and [vcpkg](https://github.com/microsoft/vcpkg) for dependencies (OpenSSL, Boost.Asio/Beast, simdjson, nlohmann-json, spdlog, zlib, GTest). `secp256k1` is fetched and built automatically via `FetchContent`.

```bash
git clone https://github.com/TuxedoFish/hyperliquid-sdk-cpp.git
cd hyperliquid-sdk-cpp

export VCPKG_ROOT=/path/to/vcpkg
cmake --preset default
cmake --build build -j$(nproc)
```

The `default` preset sets `HYPERLIQUID_BUILD_EXAMPLES=ON` and `HYPERLIQUID_BUILD_TESTS=ON`, and points `CMAKE_TOOLCHAIN_FILE` at `$VCPKG_ROOT`. To build only the library, configure manually with those options off:

```bash
cmake -B build -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake \
  -DHYPERLIQUID_BUILD_EXAMPLES=OFF -DHYPERLIQUID_BUILD_TESTS=OFF
cmake --build build -j$(nproc)
```

Run the test suite:

```bash
ctest --test-dir build
```

## Quickstart

### REST

```cpp
#include <hyperliquid/rest/RestApi.h>
#include <hyperliquid/config/Config.h>

int main() {
    hyperliquid::ApiConfig config;
    config.env = hyperliquid::Environment::Testnet;

    hyperliquid::RestApi api(config);

    auto meta = api.meta();
    for (const auto& asset : meta.universe) {
        // asset.name, asset.szDecimals, asset.maxLeverage
    }
}
```

`RestApi` methods are synchronous by default (e.g. `meta()` blocks and returns `MetaResponse`). Each has an `Async` counterpart (e.g. `metaAsync()`) that instead delivers the raw response via `RestApiListener::onMessage`, for a `RestApiListener` passed to the `RestApi(config, listener)` constructor — decode it with `RestApiMessageParser`.

### WebSocket

```cpp
#include <hyperliquid/config/Config.h>
#include <hyperliquid/websocket/WebsocketApi.h>
#include <hyperliquid/websocket/WebsocketApiListener.h>
#include <hyperliquid/websocket/WebsocketMessageHandler.h>
#include <hyperliquid/websocket/WebsocketMessageParser.h>

class BookHandler : public hyperliquid::WebsocketMessageHandler,
                     public hyperliquid::WebsocketApiListener {
public:
    void onMessage(const std::string& message) override {
        parser_.crack(message, *this);
    }
    void onConnected() override {}
    void onDisconnected(bool hasError, const std::string& errMsg) override {}

    void onL2Book(const hyperliquid::L2BookSnapshot& snapshot) override {
        // snapshot.bids / snapshot.asks, snapshot.numBids / snapshot.numAsks
    }

private:
    hyperliquid::WebsocketMessageParser parser_;
};

int main() {
    BookHandler handler;
    hyperliquid::ApiConfig config;
    config.env = hyperliquid::Environment::Testnet;

    hyperliquid::WebsocketApi ws(config, handler);
    ws.start();
    ws.subscribe(hyperliquid::SubscriptionType::L2Book, {{"coin", "BTC"}});
    // ...
    ws.stop();
}
```

More complete examples, including order placement/modification over both REST and WebSocket, are in `examples/`.

## API coverage

The tables below track which parts of the Hyperliquid API this SDK implements, cross-referenced against the current docs. This reflects `main` as of this writing — parallel work in the issue tracker is filling in gaps, so treat "not yet" as a snapshot, not a permanent gap.

Legend: ✅ implemented — ⬜ not yet implemented.

### Info endpoints (`/info`)

| Category | Endpoint (`type`) | Status | SDK method |
|---|---|---|---|
| General | `allMids` | ✅ | `RestApi::allMids` |
| General | `openOrders` | ✅ | `RestApi::openOrders` |
| General | `frontendOpenOrders` | ✅ | `RestApi::frontendOpenOrders` |
| General | `userFills` | ✅ | `RestApi::userFills` |
| General | `userFillsByTime` | ✅ | `RestApi::userFillsByTime` |
| General | `userRateLimit` | ✅ | `RestApi::userRateLimit` |
| General | `orderStatus` | ✅ | `RestApi::orderStatus` |
| General | `l2Book` | ✅ | `RestApi::l2Book` |
| General | `candleSnapshot` | ✅ | `RestApi::candleSnapshot` |
| General | `maxBuilderFee` | ✅ | `RestApi::maxBuilderFee` |
| General | `historicalOrders` | ✅ | `RestApi::historicalOrders` |
| General | `userTwapSliceFills` | ✅ | `RestApi::userTwapSliceFills` |
| General | `subAccounts` | ✅ | `RestApi::subAccounts` |
| General | `vaultDetails` | ✅ | `RestApi::vaultDetails` |
| General | `userVaultEquities` | ✅ | `RestApi::userVaultEquities` |
| General | `userRole` | ✅ | `RestApi::userRole` |
| General | `portfolio` | ✅ | `RestApi::portfolio` |
| General | `referral` | ✅ | `RestApi::referral` |
| General | `userFees` | ✅ | `RestApi::userFees` |
| General | `delegations` | ✅ | `RestApi::delegations` |
| General | `delegatorSummary` | ✅ | `RestApi::delegatorSummary` |
| General | `delegatorHistory` | ✅ | `RestApi::delegatorHistory` |
| General | `delegatorRewards` | ✅ | `RestApi::delegatorRewards` |
| General | `userDexAbstraction` | ⬜ | |
| General | `userAbstraction` | ⬜ | |
| General | `borrowLendUserState` | ✅ | `RestApi::borrowLendUserState` |
| General | `borrowLendReserveState` | ✅ | `RestApi::borrowLendReserveState` |
| General | `allBorrowLendReserveStates` | ✅ | `RestApi::allBorrowLendReserveStates` |
| General | `approvedBuilders` | ✅ | `RestApi::approvedBuilders` |
| General | `exchangeStatus` | ⬜ | |
| General | `extraAgents` | ⬜ | |
| General | `gossipPriorityAuctionStatus` | ⬜ | |
| General | `gossipRootIps` | ⬜ | |
| General | `isVip` | ⬜ | |
| General | `leadingVaults` | ⬜ | |
| General | `legalCheck` | ⬜ | |
| General | `preTransferCheck` | ⬜ | |
| General | `subAccounts2` | ⬜ | |
| General | `twapHistory` | ⬜ | |
| General | `usdcRouting` | ⬜ | |
| General | `userBorrowLendInterest` | ⬜ | |
| General | `userToMultiSigSigners` | ⬜ | |
| General | `userTwapSliceFillsByTime` | ⬜ | |
| General | `validatorL1Votes` | ⬜ | |
| General | `validatorSummaries` | ⬜ | |
| General | `vaultSummaries` | ⬜ | |
| General | `webData2` (deprecated, use `webData3`) | ⬜ | |
| Perpetuals | `perpDexs` | ✅ | `RestApi::perpDexs` |
| Perpetuals | `meta` | ✅ | `RestApi::meta` |
| Perpetuals | `metaAndAssetCtxs` | ✅ | `RestApi::metaAndAssetCtxs` |
| Perpetuals | `clearinghouseState` | ✅ | `RestApi::clearinghouseState` |
| Perpetuals | `userFunding` | ⬜ | |
| Perpetuals | `userNonFundingLedgerUpdates` | ⬜ | |
| Perpetuals | `fundingHistory` | ⬜ | |
| Perpetuals | `predictedFundings` | ✅ | `RestApi::predictedFundings` |
| Perpetuals | `perpsAtOpenInterestCap` | ✅ | `RestApi::perpsAtOpenInterestCap` |
| Perpetuals | `perpDeployAuctionStatus` | ✅ | `RestApi::perpDeployAuctionStatus` |
| Perpetuals | `activeAssetData` | ⬜ | |
| Perpetuals | `perpDexLimits` | ✅ | `RestApi::perpDexLimits` |
| Perpetuals | `perpDexStatus` | ✅ | `RestApi::perpDexStatus` |
| Perpetuals | `allPerpMetas` | ✅ | `RestApi::allPerpMetas` |
| Perpetuals | `perpAnnotation` | ✅ | `RestApi::perpAnnotation` |
| Perpetuals | `perpCategories` | ✅ | `RestApi::perpCategories` |
| Perpetuals | `perpConciseAnnotations` | ✅ | `RestApi::perpConciseAnnotations` |
| Perpetuals | `liquidatable` | ⬜ | |
| Perpetuals | `marginTable` | ⬜ | |
| Perpetuals | `maxMarketOrderNtls` | ⬜ | |
| Perpetuals | `recentTrades` | ⬜ | |
| Spot / Outcomes | `spotMeta` | ✅ | `RestApi::spotMeta` |
| Spot / Outcomes | `spotMetaAndAssetCtxs` | ✅ | `RestApi::spotMetaAndAssetCtxs` |
| Spot / Outcomes | `spotClearinghouseState` | ✅ | `RestApi::spotClearinghouseState` |
| Spot / Outcomes | `spotDeployState` | ✅ | `RestApi::spotDeployState` |
| Spot / Outcomes | `spotPairDeployAuctionStatus` | ✅ | `RestApi::spotPairDeployAuctionStatus` |
| Spot / Outcomes | `tokenDetails` | ⬜ | |
| Spot / Outcomes | `outcomeMeta` | ✅ | `RestApi::outcomeMeta` |
| Spot / Outcomes | `settledOutcome` | ✅ | `RestApi::settledOutcome` |
| Spot / Outcomes | `outcomeDeployerLimits` | ⬜ | |
| Spot / Outcomes | `outcomeTemplates` | ⬜ | |

47 of 78 documented info endpoints implemented. One (`tokenDetails`) has a `RestEndpointType` enum value reserved but no request builder or method yet.

### Exchange actions (`/exchange`)

| Action (`type`) | Status | SDK method |
|---|---|---|
| `order` | ✅ | `RestApi::placeOrder` |
| `cancel` | ✅ | `RestApi::cancelOrder` |
| `cancelByCloid` | ✅ | `RestApi::cancelOrderByCloid` |
| `scheduleCancel` | ✅ | `RestApi::scheduleCancel` |
| `modify` | ✅ | `RestApi::modifyOrder` |
| `batchModify` | ✅ | `RestApi::batchModifyOrder` |
| `updateLeverage` | ✅ | `RestApi::updateLeverage` |
| `updateIsolatedMargin` | ✅ | `RestApi::updateIsolatedMargin` |
| `approveAgent` | ✅ | `RestApi::approveAgent` |
| `agentSetAbstraction` | ✅ | `RestApi::agentSetAbstraction` |
| `approveBuilderFee` | ✅ | `RestApi::approveBuilderFee` |
| `userSetAbstraction` | ✅ | `RestApi::userSetAbstraction` |
| `sendAsset` | ✅ | `RestApi::sendAsset` |
| `agentSendAsset` | ✅ | `RestApi::agentSendAsset` |
| `sendToEvmWithData` | ✅ | `RestApi::sendToEvmWithData` |
| `usdSend` | ✅ | `RestApi::usdSend` |
| `spotSend` | ✅ | `RestApi::spotSend` |
| `withdraw3` | ✅ | `RestApi::withdraw3` |
| `usdClassTransfer` | ✅ | `RestApi::usdClassTransfer` |
| `cDeposit` | ✅ | `RestApi::cDeposit` |
| `cWithdraw` | ✅ | `RestApi::cWithdraw` |
| `tokenDelegate` | ✅ | `RestApi::tokenDelegate` |
| `vaultTransfer` | ✅ | `RestApi::vaultTransfer` |
| `hip3LiquidatorTransfer` | ✅ | `RestApi::hip3LiquidatorTransfer` |
| `twapOrder` | ✅ | `RestApi::twapOrder` |
| `twapCancel` | ✅ | `RestApi::twapCancel` |
| `reserveRequestWeight` | ✅ | `RestApi::reserveRequestWeight` |
| `noop` | ✅ | `RestApi::noop` |
| `userDexAbstraction` (deprecated, use `userSetAbstraction`) | ✅ | `RestApi::userDexAbstraction` |
| `activateOutcomeDeployer` | ⬜ | |
| `agentEnableDexAbstraction` (deprecated, use `agentSetAbstraction`) | ⬜ | |
| `authorizeAqav2Role` | ⬜ | |
| `borrowLend` | ⬜ | |
| `claimRewards` | ⬜ | |
| `convertToMultiSigUser` | ⬜ | |
| `createSubAccount` | ⬜ | |
| `createVault` | ⬜ | |
| `cSignerAction` | ⬜ | |
| `cValidatorAction` | ⬜ | |
| `evmUserModify` | ⬜ | |
| `finalizeEvmContract` | ⬜ | |
| `gossipPriorityBid` | ⬜ | |
| `linkStakingUser` | ⬜ | |
| `perpDeploy` | ⬜ | |
| `registerReferrer` | ⬜ | |
| `setDisplayName` | ⬜ | |
| `setReferrer` | ⬜ | |
| `spotDeploy` | ⬜ | |
| `spotUser` | ⬜ | |
| `stakingLinkDisableTradingUser` | ⬜ | |
| `subAccountModify` | ⬜ | |
| `subAccountSpotTransfer` | ⬜ | |
| `subAccountTransfer` | ⬜ | |
| `topUpIsolatedOnlyMargin` | ⬜ | |
| `userOutcome` | ⬜ | |
| `userPortfolioMargin` | ⬜ | |
| `validatorL1Stream` | ⬜ | |
| `vaultDistribute` | ⬜ | |
| `vaultModify` | ⬜ | |

29 of 59 documented exchange actions implemented on REST (`RestApi`). `WebsocketApi` covers a smaller subset — `placeOrder`, `cancelOrder`, `cancelOrderByCloid`, `scheduleCancel`, `modifyOrder`, `batchModifyOrder` — plus posting `meta`/`spotMeta`/`outcomeMeta`/`perpDexs` info reads over the socket; the newer transfer/staking/TWAP actions are REST-only so far.

### WebSocket subscriptions

| Channel | Status | `SubscriptionType` / callback |
|---|---|---|
| `l2Book` | ✅ | `L2Book` → `onL2Book` |
| `bbo` | ✅ | `Bbo` → `onBbo` |
| `trades` | ✅ | `Trades` → `onTrade` |
| `candle` | ✅ | `Candle` → `onCandle` |
| `allMids` | ✅ | `AllMids` → `onAllMidsEntry` |
| `activeAssetCtx` | ✅ | `ActiveAssetCtx` → `onPerpAssetCtx` / `onSpotAssetCtx` |
| `orderUpdates` | ✅ | `OrderUpdates` → `onOrderUpdate` |
| `userFills` | ✅ | `UserFills` → `onUserFill` |
| `userEvents` | ✅ | `UserEvents` → `onUserFill` / `onLiquidation` / `onNonUserCancel` |
| `userFundings` | ✅ | `UserFundings` → `onUserFundingUpdate` |
| `userNonFundingLedgerUpdates` | ✅ | `UserNonFundingLedgerUpdates` → `onLedgerUpdate` |
| `webData3` | ✅ | `WebData3` → `onWebData3` |
| `clearinghouseState` | ✅ | `ClearingHouseState` → `onClearinghouseState` |
| `openOrders` | ✅ | `OpenOrders` → `onOpenOrdersSnapshot` |
| `notification` | ✅ | `Notification` → `onNotification` |
| `twapStates` | ✅ | `TwapStates` → `onTwapStates` |
| `activeAssetData` | ✅ | `ActiveAssetData` → `onActiveAssetData` |
| `userTwapSliceFills` | ✅ | `UserTwapSliceFills` → `onUserTwapSliceFill` |
| `userTwapHistory` | ✅ | `UserTwapHistory` → `onUserTwapHistory` |
| `spotState` | ✅ | `SpotState` → `onSpotState` |
| `allDexsClearinghouseState` | ✅ | `AllDexsClearinghouseState` → `onAllDexsClearinghouseState` |
| `allDexsAssetCtxs` | ✅ | `AllDexsAssetCtxs` → `onAllDexsAssetCtxs` |
| `fastAssetCtxs` | ✅ | `FastAssetCtxs` → `onFastAssetCtx` |
| `outcomeMetaUpdates` | ✅ | `OutcomeMetaUpdates` → `onOutcomeMetaUpdate` |

24 of 24 documented channels have full typed parsing. `fastAssetCtxs` payloads are raw-DEFLATE (RFC 1951) compressed on the wire and decompressed internally (zlib) before parsing. `outcomeMetaUpdates`'s `data` is itself an array of discriminated-union entries (one of `outcomeCreated`/`outcomeSettled`/`questionUpdated`/`questionSettled` per entry) - each entry dispatches its own `onOutcomeMetaUpdate` call.

## Status

This SDK is under active development. Remaining endpoint and channel coverage is tracked in the [issue tracker](https://github.com/TuxedoFish/hyperliquid-sdk-cpp/issues).

## License

MIT — see [LICENSE](LICENSE).
