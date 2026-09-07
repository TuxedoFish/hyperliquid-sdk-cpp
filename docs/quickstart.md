# Quickstart

These are the minimal REST and WebSocket examples — enough to get a first real call working. For a build environment, see the [README's Build section](https://github.com/TuxedoFish/hyperliquid-sdk-cpp#build). For everything else the SDK can do, see the [Examples index](examples.md) and the [full API coverage tables](https://github.com/TuxedoFish/hyperliquid-sdk-cpp#api-coverage).

## REST

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

To sign and submit authenticated requests (orders, transfers, staking, ...), set `config.wallet`:

```cpp
hyperliquid::ApiConfig config;
config.env = hyperliquid::Environment::Testnet;
config.wallet = hyperliquid::Wallet{"0xYourAccountAddress", "yourPrivateKeyHex"};

hyperliquid::RestApi api(config);
```

## WebSocket

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

`WebsocketApiListener::onMessage` gives you the raw subscription message string — `crack()` it yourself (as above) to get typed callbacks per channel via `WebsocketMessageHandler`. The same `WebsocketApi` connection also sends "post" requests (the websocket equivalent of a `RestApi` call, e.g. `ws.meta(...)` or `ws.placeOrder(...)`), whose typed results arrive via `WebsocketApiListener::onPostResponse` instead.

## Putting it together: a pegged quote

A slightly bigger example showing market data and order management on the same connection: watch the best bid for a coin over the `bbo` channel, and keep a resting post-only buy order pegged to it — placing it on the first update, then re-pricing (never re-crossing the spread) whenever the best bid moves.

```cpp
#include <hyperliquid/config/Config.h>
#include <hyperliquid/websocket/WebsocketApi.h>
#include <hyperliquid/websocket/WebsocketApiListener.h>
#include <hyperliquid/websocket/WebsocketMessageHandler.h>
#include <hyperliquid/websocket/WebsocketMessageParser.h>

#include <spdlog/spdlog.h>

#include <chrono>
#include <string>
#include <thread>

class PeggedQuoter : public hyperliquid::WebsocketMessageHandler,
                      public hyperliquid::WebsocketApiListener {
public:
    // WebsocketApi's constructor needs a listener, and this listener needs to call back into
    // WebsocketApi (to place/modify orders) - attach() breaks that construction cycle.
    void attach(hyperliquid::WebsocketApi& ws) { ws_ = &ws; }

    void onMessage(const std::string& message) override {
        parser_.crack(message, *this);
    }

    void onBbo(const hyperliquid::BboUpdate& update) override {
        if (!ws_ || !update.hasBid) return;

        // update.bid.px is a string_view into the parser's internal buffer - copy it before
        // this callback returns, since it's not valid after that.
        std::string bidPx(update.bid.px);
        if (bidPx == lastPeggedPrice_) return;  // best bid hasn't moved, nothing to re-peg

        hyperliquid::OrderRequest order;
        order.asset = "ETH";
        order.isBuy = true;
        order.price = std::stod(bidPx);
        order.size = 0.01;
        order.reduceOnly = false;
        // Alo = "Add Liquidity Only" (post-only): rejected instead of filled if it would cross
        // the spread, so this never takes liquidity - it only ever rests as a passive quote.
        order.limit = hyperliquid::LimitOrderType{hyperliquid::Tif::Alo};
        order.cloid = cloid_;

        if (lastPeggedPrice_.empty()) {
            ws_->placeOrder({order}, hyperliquid::Grouping::Na);
        } else {
            // A client order id (cloid), not the exchange-assigned oid, is what makes re-pegging
            // simple here: the typed websocket post-response for order actions collapses to
            // SimpleResponse (status/type/error only, no oid) - see WebsocketApiListener::
            // onPostResponse(RestEndpointType, const SimpleResponse&, ...) below. Since we chose
            // the cloid ourselves at placement time, modifyOrder can reference it directly
            // without ever needing to parse an oid back out of a response.
            hyperliquid::ModifyRequest modify;
            modify.cloid = cloid_;
            modify.order = order;
            ws_->modifyOrder(modify);
        }
        lastPeggedPrice_ = bidPx;
    }

    void onPostResponse(hyperliquid::RestEndpointType type, const hyperliquid::SimpleResponse& resp,
                        std::optional<uint64_t>) override {
        if (resp.status != "ok")
            spdlog::warn("{} failed: {}", hyperliquid::toString(type), resp.error.value_or("unknown"));
    }

private:
    hyperliquid::WebsocketApi* ws_ = nullptr;
    hyperliquid::WebsocketMessageParser parser_;
    std::string cloid_ = hyperliquid::generateCloid();
    std::string lastPeggedPrice_;
};

int main() {
    hyperliquid::ApiConfig config;
    config.env = hyperliquid::Environment::Testnet;
    config.wallet = hyperliquid::Wallet{"0xYourAccountAddress", "yourPrivateKeyHex"};

    PeggedQuoter quoter;
    hyperliquid::WebsocketApi ws(config, quoter);
    quoter.attach(ws);

    ws.start();
    ws.subscribe(hyperliquid::SubscriptionType::Bbo, {{"coin", "ETH"}});

    std::this_thread::sleep_for(std::chrono::seconds(30));

    ws.stop();
}
```

This is a passive maker quote, not a trading strategy - it never adjusts size, never cancels on disconnect, and doesn't account for tick size or existing positions. Treat it as a starting point for combining live market data with order management, not something to point at mainnet as-is. (Compiled and run against this repo's SDK to confirm it's correct - not just an illustrative snippet.)

## Next steps

- Browse the [Examples index](examples.md) for order placement/modification, transfers, staking, vaults, and every websocket channel, over both REST and WebSocket.
- Most examples read credentials from a local `examples/test.json` file you create yourself (never committed) — see the [Examples index](examples.md#credentials) for its schema.
- Check the [full API coverage tables](https://github.com/TuxedoFish/hyperliquid-sdk-cpp#api-coverage) in the README for what's implemented and what's still outstanding.
