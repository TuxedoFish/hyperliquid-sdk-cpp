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

## Next steps

- Browse the [Examples index](examples.md) for order placement/modification, transfers, staking, vaults, and every websocket channel, over both REST and WebSocket.
- Most examples read credentials from a local `examples/test.json` file you create yourself (never committed) — see the [Examples index](examples.md#credentials) for its schema.
- Check the [full API coverage tables](https://github.com/TuxedoFish/hyperliquid-sdk-cpp#api-coverage) in the README for what's implemented and what's still outstanding.
