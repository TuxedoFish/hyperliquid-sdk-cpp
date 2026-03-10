#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "hyperliquid/signing/Wallet.h"

namespace hyperliquid {

struct Signature
{
    std::string r;
    std::string s;
    int v;
};

struct EIP712Field
{
    std::string name;
    std::string type;
};

class Signing
{
public:
    static Signature signL1Action(
        const Wallet& wallet,
        const nlohmann::ordered_json& action,
        const std::optional<std::string>& vaultAddress,
        uint64_t nonce,
        const std::optional<uint64_t>& expiresAfter,
        bool isMainnet);

    static Signature signUserSignedAction(
        const Wallet& wallet,
        const nlohmann::ordered_json& action,
        const std::vector<EIP712Field>& payloadTypes,
        const std::string& primaryType,
        bool isMainnet);

    static std::array<uint8_t, 32> actionHash(
        const nlohmann::ordered_json& action,
        const std::optional<std::string>& vaultAddress,
        uint64_t nonce,
        const std::optional<uint64_t>& expiresAfter);

    static std::string addressFromPrivateKey(const std::string& privateKey);

    static std::string toHex(const uint8_t* data, size_t length);
    static std::string toHexPadded(const uint8_t* data, size_t length);

private:
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
