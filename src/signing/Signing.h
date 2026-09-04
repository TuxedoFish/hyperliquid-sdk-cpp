#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "hyperliquid/config/Config.h"
#include "hyperliquid/types/RequestTypes.h"

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

    static nlohmann::ordered_json prepareBody(
        const ApiConfig& config,
        RestEndpointType type,
        nlohmann::ordered_json body,
        const std::optional<std::string>& vaultAddress = std::nullopt,
        const std::optional<uint64_t>& expiresAfter = std::nullopt);

    static nlohmann::ordered_json prepareApproveAgentBody(
        const ApiConfig& config,
        const std::string& agentAddress,
        const std::optional<std::string>& agentName);

    // Builds and signs the full body for EIP-712 user-signed actions (see isUserSignedAction).
    // `action` should contain every action field except hyperliquidChain/signatureChainId and the
    // nonce/time field - this fills those in (using the current unix-ms timestamp) and signs with
    // the per-type EIP-712 field list before returning {action, nonce, signature}.
    static nlohmann::ordered_json prepareUserSignedActionBody(
        const ApiConfig& config,
        RestEndpointType type,
        nlohmann::ordered_json action);
};

}
