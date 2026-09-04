#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "hyperliquid/config/Config.h"

namespace hyperliquid {

struct Signature;
struct EIP712Field;

class SigningHelpers
{
public:
    static std::array<uint8_t, 32> keccak256(const uint8_t* data, size_t length);
    static std::array<uint8_t, 32> keccak256(const std::vector<uint8_t>& data);

    static std::vector<uint8_t> hexToBytes(const std::string& hex);
    static std::string toHex(const uint8_t* data, size_t length);
    static std::string toHexPadded(const uint8_t* data, size_t length);

    static std::array<uint8_t, 32> encodeUint256(uint64_t value);
    static std::array<uint8_t, 32> encodeAddress(const std::vector<uint8_t>& addressBytes);

    static std::array<uint8_t, 32> actionHash(
        const nlohmann::ordered_json& action,
        const std::optional<std::string>& vaultAddress,
        uint64_t nonce,
        const std::optional<uint64_t>& expiresAfter);

    static std::array<uint8_t, 32> eip712Hash(
        const std::array<uint8_t, 32>& domainSeparator,
        const std::array<uint8_t, 32>& structHash);

    static std::array<uint8_t, 32> domainSeparatorHash();
    static std::array<uint8_t, 32> domainSeparatorHash(
        const std::string& name, const std::string& version, uint64_t chainId);

    static std::array<uint8_t, 32> agentStructHash(
        const std::string& source,
        const std::array<uint8_t, 32>& connectionId);

    static std::array<uint8_t, 32> userSignedStructHash(
        const std::string& primaryType,
        const std::vector<EIP712Field>& payloadTypes,
        const nlohmann::ordered_json& action);

    static Signature ecdsaSign(
        const Wallet& wallet,
        const std::array<uint8_t, 32>& messageHash);

};

}
