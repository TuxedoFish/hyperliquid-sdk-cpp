# hyperliquid-sdk-cpp

A C++23 SDK for the [Hyperliquid](https://hyperliquid.xyz) perpetuals and spot exchange API — REST (`/info`, `/exchange`) and WebSocket, with typed request/response models.

This SDK signs and submits real transactions on mainnet. Test against `Environment::Testnet` before pointing any code at `Environment::Mainnet`.

- **[Quickstart](quickstart.md)** — minimal REST and WebSocket examples to get a first call working.
- **[Examples](examples.md)** — every runnable example in `examples/`, indexed by what it demonstrates.
- **[GitHub repository](https://github.com/TuxedoFish/hyperliquid-sdk-cpp)** — build instructions, full API coverage tables, CONTRIBUTING guide, and the source itself.

## Install

Requires CMake 3.16+, a C++23 compiler, and [vcpkg](https://github.com/microsoft/vcpkg) for dependencies (OpenSSL, Boost.Asio/Beast, simdjson, nlohmann-json, spdlog, zlib, secp256k1, GTest). See the [README's Build section](https://github.com/TuxedoFish/hyperliquid-sdk-cpp#build) for the full build/install walkthrough, or [Using this library in your own project](https://github.com/TuxedoFish/hyperliquid-sdk-cpp#using-this-library-in-your-own-project) if you're consuming it via `find_package(hyperliquid-sdk CONFIG REQUIRED)`.
