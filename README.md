# hyperliquid-sdk-cpp

A C++17 SDK for the [Hyperliquid](https://hyperliquid.xyz) perpetuals and spot exchange API — REST (`/info`, `/exchange`) and WebSocket, with typed request/response models.

This SDK signs and submits real transactions on mainnet. Test against `Environment::Testnet` before pointing any code at `Environment::Mainnet`.

## Build

Requires CMake 3.16+, a C++17 compiler, and [vcpkg](https://github.com/microsoft/vcpkg) for dependencies (OpenSSL, Boost.Asio/Beast, simdjson, nlohmann-json, spdlog, GTest). `secp256k1` is fetched and built automatically via `FetchContent`.

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
| General | `frontendOpenOrders` | ⬜ | |
| General | `userFills` | ✅ | `RestApi::userFills` |
| General | `userFillsByTime` | ✅ | `RestApi::userFillsByTime` |
| General | `userRateLimit` | ⬜ | |
| General | `orderStatus` | ✅ | `RestApi::orderStatus` |
| General | `l2Book` | ✅ | `RestApi::l2Book` |
| General | `candleSnapshot` | ✅ | `RestApi::candleSnapshot` |
| General | `maxBuilderFee` | ⬜ | |
| General | `historicalOrders` | ⬜ | |
| General | `userTwapSliceFills` | ⬜ | |
| General | `subAccounts` | ⬜ | |
| General | `vaultDetails` | ⬜ | |
| General | `userVaultEquities` | ⬜ | |
| General | `userRole` | ⬜ | |
| General | `portfolio` | ⬜ | |
| General | `referral` | ⬜ | |
| General | `userFees` | ⬜ | |
| General | `delegations` | ⬜ | |
| General | `delegatorSummary` | ⬜ | |
| General | `delegatorHistory` | ⬜ | |
| General | `delegatorRewards` | ⬜ | |
| General | `userDexAbstraction` | ⬜ | |
| General | `userAbstraction` | ⬜ | |
| General | `borrowLendUserState` | ⬜ | |
| General | `borrowLendReserveState` | ⬜ | |
| General | `allBorrowLendReserveStates` | ⬜ | |
| General | `approvedBuilders` | ⬜ | |
| Perpetuals | `perpDexs` | ✅ | `RestApi::perpDexs` |
| Perpetuals | `meta` | ✅ | `RestApi::meta` |
| Perpetuals | `metaAndAssetCtxs` | ⬜ | |
| Perpetuals | `clearinghouseState` | ✅ | `RestApi::clearinghouseState` |
| Perpetuals | `userFunding` | ⬜ | |
| Perpetuals | `userNonFundingLedgerUpdates` | ⬜ | |
| Perpetuals | `fundingHistory` | ⬜ | |
| Perpetuals | `predictedFundings` | ⬜ | |
| Perpetuals | `perpsAtOpenInterestCap` | ⬜ | |
| Perpetuals | `perpDeployAuctionStatus` | ⬜ | |
| Perpetuals | `activeAssetData` | ⬜ | |
| Perpetuals | `perpDexLimits` | ⬜ | |
| Perpetuals | `perpDexStatus` | ⬜ | |
| Perpetuals | `allPerpMetas` | ⬜ | |
| Perpetuals | `perpAnnotation` | ⬜ | |
| Perpetuals | `perpCategories` | ⬜ | |
| Perpetuals | `perpConciseAnnotations` | ⬜ | |
| Spot / Outcomes | `spotMeta` | ✅ | `RestApi::spotMeta` |
| Spot / Outcomes | `spotMetaAndAssetCtxs` | ⬜ | |
| Spot / Outcomes | `spotClearinghouseState` | ⬜ | |
| Spot / Outcomes | `spotDeployState` | ⬜ | |
| Spot / Outcomes | `spotPairDeployAuctionStatus` | ⬜ | |
| Spot / Outcomes | `tokenDetails` | ⬜ | |
| Spot / Outcomes | `outcomeMeta` | ✅ | `RestApi::outcomeMeta` |
| Spot / Outcomes | `settledOutcome` | ⬜ | |
| Spot / Outcomes | `outcomeDeployerLimits` | ⬜ | |

12 of 55 documented info endpoints implemented. A handful (`metaAndAssetCtxs`, `userRateLimit`, `spotMetaAndAssetCtxs`, `spotClearinghouseState`, `spotDeployState`, `spotPairDeployAuctionStatus`, `tokenDetails`) have a `RestEndpointType` enum value reserved but no request builder or method yet.

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
| `approveAgent` | ⬜ | |
| `approveBuilderFee` | ⬜ | |
| `sendAsset` | ⬜ | |
| `agentSendAsset` | ⬜ | |
| `sendToEvmWithData` | ⬜ | |
| `usdSend` | ⬜ | |
| `spotSend` | ⬜ | |
| `withdraw3` | ⬜ | |
| `usdClassTransfer` | ⬜ | |
| `cDeposit` | ⬜ | |
| `cWithdraw` | ⬜ | |
| `tokenDelegate` | ⬜ | |
| `vaultTransfer` | ⬜ | |
| `hip3LiquidatorTransfer` | ⬜ | |
| `twapOrder` | ⬜ | |
| `twapCancel` | ⬜ | |
| `reserveRequestWeight` | ⬜ | |
| `noop` | ⬜ | |
| `userDexAbstraction` (deprecated) | ⬜ | |

8 of 27 documented exchange actions implemented. Both REST (`RestApi`) and WebSocket (`WebsocketApi`) expose the same 8 — the WebSocket API additionally supports posting `meta`/`spotMeta`/`outcomeMeta`/`perpDexs` info reads over the socket.

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
| `notification` | ⬜ | `Notification` enum exists; subscribable, response not yet parsed |
| `twapStates` | ⬜ | `TwapStates` enum exists; subscribable, response not yet parsed |
| `activeAssetData` | ⬜ | `ActiveAssetData` enum exists; subscribable, response not yet parsed |
| `userTwapSliceFills` | ⬜ | `UserTwapSliceFills` enum exists; subscribable, response not yet parsed |
| `userTwapHistory` | ⬜ | `UserTwapHistory` enum exists; subscribable, response not yet parsed |
| `spotState` | ⬜ | |
| `allDexsClearinghouseState` | ⬜ | |
| `allDexsAssetCtxs` | ⬜ | |
| `outcomeMetaUpdates` | ⬜ | |
| `fastAssetCtxs` | ⬜ | |

14 of 24 documented channels have full typed parsing. Five more (`notification`, `twapStates`, `activeAssetData`, `userTwapSliceFills`, `userTwapHistory`) can be subscribed to — the server will stream them — but the SDK doesn't yet decode the payload into a typed callback; unrecognized messages are logged and dropped. The remaining five channels have no subscription support at all yet.

## Status

This SDK is under active development. Remaining endpoint and channel coverage is tracked in the [issue tracker](https://github.com/TuxedoFish/hyperliquid-sdk-cpp/issues).

## License

MIT — see [LICENSE](LICENSE).
