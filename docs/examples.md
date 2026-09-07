# Examples

Every runnable example lives in [`examples/`](https://github.com/TuxedoFish/hyperliquid-sdk-cpp/tree/main/examples) and is built automatically when `HYPERLIQUID_BUILD_EXAMPLES=ON` (the default `cmake --preset default` config). Binaries land in `build/` named after the source file, e.g. `examples/rest_orders.cpp` builds `build/rest_orders`.

Examples are paired: most `/exchange` or `/info` call in `rest_*.cpp` has a `ws_*.cpp` counterpart making the same call over the websocket "post" mechanism instead of plain HTTP.

## Credentials

Most examples read credentials from `examples/test.json`, a local file you create yourself — it's gitignored and never committed, and the repo doesn't ship one. Copy the template at [`examples/example.json`](https://github.com/TuxedoFish/hyperliquid-sdk-cpp/blob/main/examples/example.json) to `examples/test.json` and fill it in, or create it directly:

```json
{
  "wallet": "0xYourAccountAddress",
  "privateKey": "yourPrivateKeyHex",
  "subaccount": "0xOptionalSubaccountAddress"
}
```

`wallet`/`privateKey` are required; `subaccount` is only read by the examples that need one (see [`ws_subaccount`](#subscription-channels) below). Point this at a **testnet** wallet while trying things out — these examples sign and submit real requests.

## REST examples

### Market data & metadata

| Example | Demonstrates |
|---|---|
| `rest_meta.cpp` | `meta`, `spotMeta`, `outcomeMeta`, `perpDexs` — top-level metadata for perps, spot, prediction markets, and builder-deployed dexes |
| `rest_info.cpp` | `allMids`, `l2Book`, `candleSnapshot`, `clearinghouseState`, `openOrders`, `userFills` — core market data and account info |
| `rest_metadata.cpp` | `metaAndAssetCtxs`, `spotMetaAndAssetCtxs`, `spotClearinghouseState`, `frontendOpenOrders`, `historicalOrders`, `subAccounts`, `userFees`, `userTwapSliceFills`, `maxBuilderFee`, `approvedBuilders` — broader account/market metadata |
| `rest_perp_info.cpp` | `allPerpMetas`, `perpAnnotation`, `perpCategories`, `perpConciseAnnotations`, `perpsAtOpenInterestCap`, `predictedFundings` — perp-specific metadata |
| `rest_recent_trades.cpp` | `recentTrades` |
| `rest_funding_history.cpp` | `fundingHistory`, `userFunding`, `userNonFundingLedgerUpdates` |
| `rest_user_rate_limit.cpp` | `userRateLimit` |

### Orders & positions

| Example | Demonstrates |
|---|---|
| `rest_orders.cpp` | `placeOrder`, `modifyOrder`, `cancelOrder`, `cancelOrderByCloid` |
| `rest_leverage.cpp` | `updateLeverage`, `updateIsolatedMargin`, plus `l2Book`/`placeOrder` for context |
| `rest_schedule_cancel.cpp` | `scheduleCancel` — the exchange-side dead man's switch |
| `rest_twap.cpp` | `twapOrder`, `twapCancel` |
| `rest_twap_slices.cpp` | `twapOrder`, `twapCancel`, `userTwapSliceFills`, `clearinghouseState` — a full TWAP lifecycle with slice-fill tracking |

### Transfers, staking & vaults

| Example | Demonstrates |
|---|---|
| `rest_transfers.cpp` | `usdSend`, `usdClassTransfer`, `spotSend`, `sendAsset`, `withdraw3` |
| `rest_vault_transfer.cpp` | `vaultTransfer` |
| `rest_vaults.cpp` | `vaultDetails`, `userVaultEquities`, `portfolio`, `referral`, `userRole` |
| `rest_staking.cpp` | `cDeposit`, `cWithdraw`, `tokenDelegate`, `delegations`, `delegatorSummary`, `delegatorHistory`, `delegatorRewards` |
| `rest_borrow_lend.cpp` | `borrowLendUserState`, `borrowLendReserveState`, `allBorrowLendReserveStates` |
| `rest_borrow_lend_action.cpp` | `borrowLend` |

### Agents & account abstraction

| Example | Demonstrates |
|---|---|
| `rest_approve_agent.cpp` | `approveAgent` — register an agent/API wallet |
| `rest_agent_set_abstraction.cpp` | The full agent flow: generate a key with `generateAgentPrivateKey`, `approveAgent` it against the main account, then sign `agentSetAbstraction` with the agent's own key |
| `rest_approve_builder_fee.cpp` | `approveBuilderFee` |
| `rest_user_abstraction.cpp` | `userAbstraction`, `userDexAbstractionState` |
| `rest_user_dex_abstraction.cpp` | `userDexAbstraction` |
| `rest_user_set_abstraction.cpp` | `userSetAbstraction` |
| `rest_misc_actions.cpp` | `noop`, `reserveRequestWeight`, `agentSendAsset`, `sendToEvmWithData` |

### Deploys, HIP-3 & prediction markets

| Example | Demonstrates |
|---|---|
| `rest_spot_deploy.cpp` | `spotDeployState`, `spotPairDeployAuctionStatus` |
| `rest_spot_deploy_action.cpp` | `spotDeployRegisterToken2`, `spotDeployUserGenesis`, `spotDeployGenesis`, `spotDeployRegisterSpot`, `spotDeployRegisterHyperliquidity` — the full spot token deploy sequence |
| `rest_perp_deploy_action.cpp` | `perpDeployRegisterAsset2` |
| `rest_hip3_deployer.cpp` | `perpDeployAuctionStatus`, `perpDexLimits`, `perpDexStatus` — builder-deployed perp dex info |
| `rest_hip3_liquidator_transfer.cpp` | `hip3LiquidatorTransfer` |
| `rest_settled_outcome.cpp` | `settledOutcome` — prediction market resolution |

## WebSocket examples

Every REST example above that maps to an `/exchange` or authenticated `/info` action has a `ws_*.cpp` counterpart sending the same request as a websocket "post" instead (e.g. `rest_orders.cpp` ↔ `ws_orders.cpp`'s post-request calls) - see [`WebsocketApi`](https://github.com/TuxedoFish/hyperliquid-sdk-cpp/blob/main/include/hyperliquid/websocket/WebsocketApi.h) for the full method list. This section covers the ones with something distinct to show, plus every subscription-channel example.

### Post requests over websocket

| Example | Demonstrates |
|---|---|
| `ws_info.cpp` | `allMids`, `l2Book`, `candleSnapshot`, `clearinghouseState`, `openOrders`, `orderStatus`, `userFills`, `userFillsByTime` over websocket post |
| `ws_metadata.cpp` | `metaAndAssetCtxs`, `spotMetaAndAssetCtxs`, `spotClearinghouseState`, `frontendOpenOrders`, `historicalOrders`, `subAccounts`, `userFees`, `maxBuilderFee`, `approvedBuilders` |
| `ws_leverage.cpp` | `updateLeverage`, `updateIsolatedMargin` |
| `ws_twap.cpp` | `twapOrder`, `twapCancel` |
| `ws_transfers.cpp` | `usdSend`, `usdClassTransfer`, `spotSend`, `sendAsset`, `withdraw3` |
| `ws_vaults.cpp` | `vaultDetails`, `userVaultEquities`, `portfolio`, `referral`, `userRole` |
| `ws_vault_transfer.cpp` | `vaultTransfer` |
| `ws_staking.cpp` | `cDeposit`, `cWithdraw`, `tokenDelegate`, `delegations`, `delegatorSummary`, `delegatorHistory`, `delegatorRewards` |
| `ws_borrow_lend.cpp` | `borrowLendUserState`, `borrowLendReserveState`, `allBorrowLendReserveStates` |
| `ws_borrow_lend_action.cpp` | `borrowLend` |
| `ws_approve_agent.cpp` | `approveAgent` |
| `ws_agent_set_abstraction.cpp` | `agentSetAbstraction` |
| `ws_approve_builder_fee.cpp` | `approveBuilderFee` |
| `ws_user_abstraction.cpp` | `userAbstraction`, `userDexAbstractionState` |
| `ws_user_dex_abstraction.cpp` | `userDexAbstraction` |
| `ws_user_set_abstraction.cpp` | `userSetAbstraction` |
| `ws_user_rate_limit.cpp` | `userRateLimit` |
| `ws_misc_actions.cpp` | `noop`, `reserveRequestWeight`, `agentSendAsset`, `sendToEvmWithData` |
| `ws_perp_info.cpp` | `allPerpMetas`, `perpAnnotation`, `perpCategories`, `perpConciseAnnotations`, `perpsAtOpenInterestCap`, `predictedFundings` |
| `ws_recent_trades.cpp` | `recentTrades` |
| `ws_funding_history.cpp` | `fundingHistory`, `userFunding`, `userNonFundingLedgerUpdates` |
| `ws_hip3_deployer.cpp` | `perpDeployAuctionStatus`, `perpDexLimits`, `perpDexStatus` |
| `ws_hip3_liquidator_transfer.cpp` | `hip3LiquidatorTransfer` |
| `ws_perp_deploy_action.cpp` | `perpDeployRegisterAsset2` |
| `ws_spot_deploy.cpp` | `spotDeployState`, `spotPairDeployAuctionStatus` |
| `ws_spot_deploy_action.cpp` | `spotDeployRegisterToken2`, `spotDeployUserGenesis`, `spotDeployGenesis`, `spotDeployRegisterSpot`, `spotDeployRegisterHyperliquidity` |
| `ws_settled_outcome.cpp` | `settledOutcome` |

### Subscription channels

| Example | Demonstrates |
|---|---|
| `ws_book.cpp` | Subscribes `l2Book`, `bbo`, `trades` for one coin — the core market-data channels |
| `ws_orders.cpp` | Subscribes `orderUpdates`, `userFills`, `activeAssetCtx`, then **unsubscribes** and re-subscribes — the full subscription lifecycle |
| `ws_fills.cpp` | Subscribes `orderUpdates`, `userFills`, `activeAssetCtx` |
| `ws_account_channels.cpp` | Subscribes `userFundings`, `userNonFundingLedgerUpdates`, `webData3`, `clearinghouseState`, `openOrders` — account-level snapshot/update channels |
| `ws_new_channels.cpp` | Subscribes `twapStates`, `notification`, `userTwapSliceFills`, `userTwapHistory`, `activeAssetData`, `spotState`, `allDexsClearinghouseState`, `allDexsAssetCtxs`, `fastAssetCtxs` |
| `ws_subaccount.cpp` | Subscribes `orderUpdates` filtered to a subaccount address (read from `test.json`'s optional `subaccount` field) |
| `ws_outcomes.cpp` | Subscribes `orderUpdates` for a prediction market |
| `ws_outcomes_book.cpp` | Subscribes `l2Book`, `trades` for a prediction market |
| `ws_outcomes_fills.cpp` | Subscribes `orderUpdates`, `userFills` for a prediction market |
| `ws_outcome_meta_updates.cpp` | Subscribes `outcomeMetaUpdates` — prediction market metadata changes |
