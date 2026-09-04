#pragma once

#include <string>

namespace hyperliquid {

// Generates a new random secp256k1 private key (hex, 0x-prefixed) suitable for creating a
// new Hyperliquid agent (API) wallet to pass to RestApi::approveAgent. Unrelated to
// order/action nonce generation.
std::string generateAgentPrivateKey();

// Derives the Ethereum-style address (hex, 0x-prefixed, 20 bytes) for a given private key.
std::string privateKeyToAddress(const std::string& privateKeyHex);

}
