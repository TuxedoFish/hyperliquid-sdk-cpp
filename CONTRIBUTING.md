# Contributing

Thanks for looking at this project. Most open issues are scoped around adding a single missing API surface (an `/info` endpoint, an `/exchange` action, or a websocket channel) and follow a repeatable pattern - this doc walks through it.

## Build first

Follow the [Build section of the README](README.md#build). Confirm `cmake --build build -j$(nproc)` and `ctest --test-dir build` both pass on a clean checkout before making any changes, so you know a later failure is something you introduced.

## Adding a new `/info` endpoint

Worked example: [#36](https://github.com/TuxedoFish/hyperliquid-sdk-cpp/pull/36) added `vaultDetails`, `userVaultEquities`, `portfolio`, `referral`, `userRole`.

1. **`include/hyperliquid/types/RequestTypes.h`** - add the endpoint to the `RestEndpointType` enum, and a case each in `toString()` and `isAuthenticated()`.
2. **`include/hyperliquid/types/ResponseTypes.h`** - add the response struct(s) for the parsed result.
3. **`src/messages/InfoRequestBuilder.h`/`.cpp`** - add a static method building the request body (`{"type": "...", ...params}`).
4. **`include/hyperliquid/rest/RestApiMessageParser.h`** + **`src/rest/RestApiMessageParser.cpp`** - add a `parseX` method that turns the raw JSON response into your response struct.
5. **`include/hyperliquid/rest/RestEndpointListener.h`** - add an `onX` virtual callback (used by the async/listener path).
6. **`include/hyperliquid/rest/RestApi.h`** + **`src/rest/RestApi.cpp`** - add the public `RestApi::x(...)` method (sync) wiring the builder → send → parser together. Also wire the corresponding `xAsync(...)` method and its `case` in the async dispatch switch.
7. Add tests in `tests/rest_info_test.cpp` (or a new `tests/rest_<feature>_test.cpp` if the endpoint set is large enough to warrant its own file, as `rest_vaults_test.cpp` did) - both a request-builder test and a response-parser test per endpoint. Register the new test file in `CMakeLists.txt` if it's a new file.
8. Add or extend an example in `examples/` that calls the real endpoint against testnet, and update the README's coverage table.

### JSON parsing notes (read this before you write a parser)

This codebase uses simdjson's `ondemand` API, which has real pitfalls that have caused genuine bugs in past PRs:

- **Numbers are often string-encoded on the wire** (e.g. `"sz": "0.0013"`, not `"sz": 0.0013`) to avoid float precision issues. Don't assume `.get_double()` will work - use `parseNumberField(obj, "key")` or `toDoubleField(obj, "key")` (both already handle the string-or-number case), or check the raw payload directly if you're not sure.
- **Fields must be read in the order they appear in the JSON.** Speculatively probing for a field that isn't present (e.g. checking for `"code"` before `"required"` when only one of the two exists) consumes the rest of the object during the scan, and later field reads on the same object will then fail even though the field is genuinely there. If a field's presence depends on some other field's value (a discriminated union), branch on the discriminant first and only read the fields that branch actually has.
- **Don't trust field-shape assumptions from memory, docs, or a previous implementation without checking a real payload.** Several real bugs in this codebase were exactly this: a comment or original implementation assumed one JSON shape, and the live wire format was different (nested differently, an extra level of array nesting, a field that doesn't always exist). Where practical, run your new endpoint against testnet and look at the actual response before assuming your parser is done - unit tests using fabricated JSON will happily pass against the wrong assumption.

## Adding a new `/exchange` action

Same shape as above, but touching `ExchangeRequestBuilder.h`/`.cpp` instead of `InfoRequestBuilder`, and note whether the action needs `RequestTypes.h`'s `isUserSignedAction()` switch (EIP-712 user-signed actions, like transfers/staking) versus the default L1 action signing path.

## Adding a new websocket subscription channel

Worked example: [#39](https://github.com/TuxedoFish/hyperliquid-sdk-cpp/pull/39) added 9 channels including `fastAssetCtxs`.

1. **`include/hyperliquid/types/RequestTypes.h`** - add the channel to `SubscriptionType`, plus its `toString()`/`stringToSubscriptionType()` cases.
2. **`include/hyperliquid/types/ResponseTypes.h`** - add the response struct for the channel's payload.
3. **`include/hyperliquid/websocket/WebsocketMessageHandler.h`** - add an `onX(...)` virtual callback.
4. **`src/websocket/WebsocketMessageParser.cpp`** - add a `channel == "x"` branch in the dispatch, and a `crackX(...)` function that parses the payload and calls the callback.
5. If the channel needs a filter parameter beyond `user` (like `coin` for `activeAssetData`), check `WebsocketApi::isUserSubscription()` in `src/websocket/WebsocketApi.cpp` covers it correctly.
6. Genuinely stateless, reusable parsing helpers (not tied to a specific channel's response shape) belong in `src/websocket/WebsocketParsingUtils.h`/`.cpp`, not inline in the parser - see that file for the existing base64/DEFLATE helpers used by `fastAssetCtxs`.
7. Add parser tests in `tests/websocket_parser_test.cpp`, an example subscribing to the channel, and update the README.

## Pull requests

- Run the full test suite locally before opening a PR; CI will re-run it, but don't rely on CI to catch what a two-minute local run would have.
- If you can run your change against testnet, do it and paste the real output in the PR description - several real bugs in this codebase (wrong field types, wrong array nesting, wrong discriminated-union assumptions) were only caught this way, not by unit tests against fabricated fixtures.
- If a field's real shape is genuinely undocumented and you're inferring it, say so explicitly in the PR rather than presenting it as confirmed - see recent PR descriptions for the tone this project uses ("please double-check before relying on this").
- Small, focused PRs (one endpoint group, one channel set) are easier to review than large ones spanning unrelated areas.
