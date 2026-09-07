# Security Policy

## Scope

This repository is a client SDK: it builds, signs, and sends requests to the Hyperliquid API. A vulnerability here could mean things like a malformed or malicious server response being parsed unsafely, a signing/key-handling bug that produces an incorrect or leaked signature, or a dependency vulnerability - anything that could lead to loss of funds, exposure of a private key, or incorrect execution of a trade is treated as a security issue, not a regular bug.

Vulnerabilities in the Hyperliquid exchange or protocol itself (as opposed to this SDK's implementation) are out of scope for this repository - please report those directly to Hyperliquid.

## Supported Versions

This project is pre-1.0, so only the most recent tagged release and the latest commit on `main` are supported. Please make sure you can reproduce an issue against one of those before reporting.

| Version       | Supported          |
| ------------- | ------------------ |
| main          | :white_check_mark: |
| latest tag    | :white_check_mark: |
| older tags    | :x:                |

## Reporting a Vulnerability

**Please do not open a public GitHub issue for security vulnerabilities.**

Use [GitHub's private vulnerability reporting](https://github.com/TuxedoFish/hyperliquid-sdk-cpp/security/advisories/new) for this repository (see the **Security** tab). This opens a private advisory visible only to the maintainer until a fix is ready, and lets you collaborate on a patch before public disclosure.

Please include:

- A description of the vulnerability and its potential impact.
- Steps to reproduce, or a minimal proof-of-concept.
- The affected version/commit.

You should expect an initial response within a few days. If the report is accepted, we'll work with you on a fix and coordinate a disclosure timeline; if it's declined, we'll explain why. Please give us a reasonable amount of time to address the issue before any public disclosure.
