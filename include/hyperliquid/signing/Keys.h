#pragma once

#include <string>

namespace hyperliquid {

// Generates a new random secp256k1 private key (hex, no 0x prefix), suitable for an agent/API
// wallet you then register on-chain via RestApi::approveAgent/WebsocketApi::approveAgent before
// using it to sign requests on an account's behalf.
std::string generateAgentPrivateKey();

// Derives the 0x-prefixed Ethereum-style address for a private key (hex, with or without 0x
// prefix). Use this to find the address to pass as agentAddress when approving a key generated
// by generateAgentPrivateKey.
std::string privateKeyToAddress(const std::string& privateKeyHex);

}
