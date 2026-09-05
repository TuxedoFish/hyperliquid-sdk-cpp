# Thread safety

This document describes the actual, audited thread-safety behavior of `RestApi` and
`WebsocketApi`. It reflects what the current implementation does, not an aspirational
guarantee — where the implementation is unsynchronized, that is stated explicitly rather
than left ambiguous.

## `RestApi`

**Safe:** calling any mix of sync (`meta()`, `placeOrder()`, ...) and async
(`metaAsync()`, `placeOrderAsync()`, ...) methods on the same `RestApi` instance,
concurrently, from any number of threads.

Each call builds and signs its own request body, then hands it to a freshly constructed
`HttpSession` (`src/rest/RestApi.cpp:94`, `:124`) that owns its own socket, buffers, and
request/response objects — there is no per-call shared mutable state to race on. Body
construction (`Impl::prepareBodyForType`, `src/rest/RestApi.cpp:67`) and signing
(`Signing::prepareBody` et al., `src/signing/Signing.cpp`) run synchronously on the
calling thread and touch only locals, the read-only `ApiConfig`, and the read-only
`ExchangeRequestBuilder`'s symbol map. The symbol map (`SymbolMap`,
`src/rest/SymbolMap.h`) is populated once, synchronously, during `RestApi` construction
(`src/rest/RestApi.cpp:148-158`) and only ever read afterwards, so concurrent reads from
multiple threads post-construction are safe.

All HTTP I/O runs on a single `boost::asio::io_context` driven by exactly one dedicated
background thread (`RestApi::Impl::thread`, `src/rest/RestApi.cpp:52`). Consequently every
`RestApiListener` callback (`onMessage`, `onError`, `onRateLimitExceeded`) is invoked
serially from that one thread, never concurrently with another callback, regardless of
how many application threads issued the originating requests. Implement `RestApiListener`
accordingly — it will never be re-entered concurrently by the SDK itself, but it will be
called from a thread the application did not create.

**Caveat — nonce collisions under concurrent signing:** every authenticated request's
nonce is computed independently as the current millisecond-resolution wall-clock time on
the calling thread, with no shared counter or lock (`src/signing/Signing.cpp:63-66`,
`:134-136`, `:259-261`). Two authenticated calls issued at (or near) the same millisecond
— whether from two different threads or in rapid succession on one thread — can compute
the same nonce. Hyperliquid requires strictly increasing nonces per user, so one of the
two requests may then be rejected by the exchange. This is not a memory-safety issue in
the SDK, but it is a real correctness hazard for anyone issuing exchange actions
concurrently or in a tight loop, and it is not currently mitigated (e.g. no monotonic
per-`RestApi`-instance nonce counter).

Not evaluated here: safety of constructing/destroying multiple `RestApi` instances
concurrently, or of destroying a `RestApi` instance while other threads are still calling
into it — do not do this.

## `WebsocketApi`

**Safe:** `subscribe()` and `unsubscribe()` may be called from any thread at any time
after `start()`. Both only ever call `WebsocketRunner::send()`, which posts the write onto
the connection's `io_context` via `net::post` (`src/websocket/WebsocketRunner.cpp:33-41`)
rather than touching the socket or write queue directly — so the actual queue/socket state
is only ever touched from the single internal I/O thread that runs the connection's
`io_context` (spawned in `WebsocketApi::start()`, `src/websocket/WebsocketApi.cpp:207`).
`WebsocketApiListener` callbacks (`onMessage`, `onConnected`, `onDisconnected`) are
likewise only ever invoked from that one internal thread, serially.

**Not safe — data race on `postRequestInfo`:** the "post" request/response correlation
methods — `spotMeta()`, `meta()`, `outcomeMeta()`, `perpDexs()`, `placeOrder()`,
`cancelOrder()`, `cancelOrderByCloid()`, `scheduleCancel()`, `modifyOrder()`,
`batchModifyOrder()` — do **not** go through the safe `send()` posting path for their own
bookkeeping. `Impl::signAndSend` writes directly into
`std::unordered_map<uint64_t, PostRequestInfo> postRequestInfo`
(`src/websocket/WebsocketApi.cpp:33`, write at `:116`) on whatever thread calls the public
method. That same map is read and erased from in `Impl::onWsMessage`
(`src/websocket/WebsocketApi.cpp:73-77`), which runs on the internal I/O thread whenever a
`"post"`-channel response arrives. Neither side takes a lock, and no strand/post is used
to serialize the two.

This is a genuine, unmitigated data race, not just an absence of a documented guarantee.
It also affects the SDK's ordinary intended usage pattern: `start()` is non-blocking — it
spawns the I/O thread and returns immediately (`src/websocket/WebsocketApi.cpp:205-211`)
— so a normal caller who invokes `start()` and then calls `placeOrder()` from its own
(e.g. main/strategy) thread is already running with two threads touching
`postRequestInfo` concurrently, with no extra multi-threading on the application's part
required to hit it. A concrete failure mode: a post response for an earlier
`placeOrder()` call arrives and is being processed (`onWsMessage` calling
`postRequestInfo.find()`/`.erase()` on the I/O thread) at the same moment the
application's thread calls `cancelOrder()`, which performs `postRequestInfo[id] = ...`
(an insert, which may rehash the table). Concurrent insert/erase on a
`std::unordered_map` without external synchronization is undefined behavior in C++, and
can manifest as a crash, a corrupted map, or a silently dropped/misdelivered
correlation-ID response. This is likely to be intermittent and load-dependent rather than
immediately reproducible.

A secondary, lower-severity instance of the same pattern: `WebsocketApi::stop()` sets the
plain `bool Impl::stopping` (`src/websocket/WebsocketApi.cpp:28`, write at `:215`) from
the calling thread, while `onWsMessage` reads it (`:53`) on the I/O thread, again without
synchronization.

Neither issue is fixed as part of this documentation change — see the accompanying PR
description for a suggested follow-up.

**Until fixed, the recommended safe usage pattern is:** call `start()`, then only ever
call `spotMeta()`, `meta()`, `outcomeMeta()`, `perpDexs()`, `placeOrder()`,
`cancelOrder()`, `cancelOrderByCloid()`, `scheduleCancel()`, `modifyOrder()`, and
`batchModifyOrder()` from a single application thread (any one thread is fine, it does
not need to be the thread that called `start()`), and do not call them concurrently with
each other from multiple threads. `subscribe()`/`unsubscribe()` do not need this
restriction.

**Not evaluated here:** safety of constructing/destroying multiple `WebsocketApi`
instances concurrently, or of calling `stop()` concurrently with in-flight
`start()`/other calls beyond the `stopping` race noted above — avoid overlapping these.
