# Changelog

All notable changes to this project are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.1.0] - Unreleased

Initial release.

### Added

- **REST API** (`RestApi`): 53 of 78 documented `/info` endpoints and 32 of 59 documented
  `/exchange` actions, each with a synchronous method and an `Async` + `RestApiListener`
  counterpart. Covers market data, account/portfolio state, order placement/cancellation/
  modification, leverage and margin management, transfers (USD/spot/EVM), vaults, staking
  and delegation, sub-accounts, borrow/lend, HIP-3 perp/spot deployment, and prediction
  markets. See the README's coverage tables for the full endpoint-by-endpoint breakdown.
- **WebSocket API** (`WebsocketApi`, `WebsocketMessageParser`): full typed parsing for all 24
  documented subscription channels, plus a `post` mechanism that can carry any info request or
  signed exchange action over the socket with typed response dispatch.
- **Signing**: EIP-712 typed-data signing for both L1 actions and user-signed actions
  (transfers, staking, agent approval), shared between REST and WebSocket transports.
- **Typed request/response models** throughout, replacing raw JSON with `struct`s and enums
  for every implemented endpoint and channel.
- Worked examples for both transports under `examples/`, most verified live against testnet.
- CI: build matrix across GCC/Clang/macOS, AddressSanitizer + UndefinedBehaviorSanitizer,
  code coverage via Codecov, CodeQL static analysis, OSV-Scanner dependency scanning, and a
  scheduled libFuzzer campaign against both message parsers.
- Packaging: CMake install/export targets (`find_package(hyperliquid-sdk)`) and a vcpkg
  manifest for consuming this SDK's own dependencies.
- `SECURITY.md`, `CODE_OF_CONDUCT.md`, and `CONTRIBUTING.md` documenting the vulnerability
  reporting process, community standards, and the repeatable pattern for adding new
  endpoints/channels.
