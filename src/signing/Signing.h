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

    // vaultAddress/expiresAfter are folded into the L1 action hash (see SigningHelpers::actionHash)
    // for endpoints signed with signL1Action; nonce is the current unix-ms timestamp and must be
    // strictly increasing per-user, so don't reuse or backdate it. expiresAfter is an optional
    // unix-ms deadline after which the exchange should reject the action even if it arrives late;
    // omit it (the default) unless you specifically need that guarantee.
    static nlohmann::ordered_json prepareBody(
        const ApiConfig& config,
        RestEndpointType type,
        nlohmann::ordered_json body,
        const std::optional<std::string>& vaultAddress = std::nullopt,
        const std::optional<uint64_t>& expiresAfter = std::nullopt);

    // approveAgent is a "user-signed action" (EIP-712 typed data via signUserSignedAction), not
    // an L1 action, so it doesn't go through prepareBody's vaultAddress/expiresAfter handling.
    static nlohmann::ordered_json prepareApproveAgentBody(
        const ApiConfig& config,
        const std::string& agentAddress,
        const std::optional<std::string>& agentName);
};

}
