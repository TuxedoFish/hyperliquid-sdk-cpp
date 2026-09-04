#include "Signing.h"
#include "SigningHelpers.h"

#include <chrono>
#include <spdlog/spdlog.h>

namespace hyperliquid {

nlohmann::ordered_json Signing::prepareBody(
    const ApiConfig& config,
    RestEndpointType type,
    nlohmann::ordered_json body,
    const std::optional<std::string>& vaultAddress,
    const std::optional<uint64_t>& expiresAfter)
{
    if (vaultAddress) body["vaultAddress"] = *vaultAddress;
    if (expiresAfter) body["expiresAfter"] = *expiresAfter;

    if (isAuthenticated(type))
    {
        if (!config.wallet.has_value())
        {
            spdlog::error("Wallet not configured, can't send authenticated request: {}", toString(type));
            return body;
        }
        // Nonce: current unix time in ms. Must be strictly increasing per-user (and within the
        // exchange's accepted clock window), so this relies on the local clock being reasonably
        // accurate and monotonic across requests - don't override it with a custom generator.
        uint64_t nonce = static_cast<uint64_t>(
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count());
        body["nonce"] = nonce;

        bool isMainnet = (config.env == Environment::Mainnet);
        auto action = body["action"];
        auto signature = signL1Action(
            config.wallet.value(), action, vaultAddress, nonce, expiresAfter, isMainnet);

        nlohmann::ordered_json signatureJson;
        signatureJson["r"] = signature.r;
        signatureJson["s"] = signature.s;
        signatureJson["v"] = signature.v;
        body["signature"] = signatureJson;
    }

    return body;
}

Signature Signing::signUserSignedAction(
    const Wallet& wallet,
    const nlohmann::ordered_json& action,
    const std::vector<EIP712Field>& payloadTypes,
    const std::string& primaryType,
    bool isMainnet)
{
    uint64_t chainId = 0x66eee;

    auto structHash = SigningHelpers::userSignedStructHash(primaryType, payloadTypes, action);
    auto domSep = SigningHelpers::domainSeparatorHash("HyperliquidSignTransaction", "1", chainId);
    auto finalHash = SigningHelpers::eip712Hash(domSep, structHash);

    return SigningHelpers::ecdsaSign(wallet, finalHash);
}

nlohmann::ordered_json Signing::prepareApproveAgentBody(
    const ApiConfig& config,
    const std::string& agentAddress,
    const std::optional<std::string>& agentName)
{
    nlohmann::ordered_json body;

    if (!config.wallet.has_value())
    {
        spdlog::error("Wallet not configured, can't send authenticated request: {}",
                      toString(RestEndpointType::ApproveAgent));
        return body;
    }

    uint64_t nonce = static_cast<uint64_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count());
    bool isMainnet = (config.env == Environment::Mainnet);

    nlohmann::ordered_json action;
    action["type"] = "approveAgent";
    action["agentAddress"] = agentAddress;
    // Signed with an empty string when no name is given (matching the reference SDKs), then
    // stripped from the outgoing body below - the exchange treats a missing agentName the same
    // as an empty one when it recomputes the signed struct hash for verification.
    action["agentName"] = agentName.value_or("");
    action["nonce"] = nonce;
    action["signatureChainId"] = "0x66eee";
    action["hyperliquidChain"] = isMainnet ? "Mainnet" : "Testnet";

    auto signature = signUserSignedAction(
        config.wallet.value(), action,
        {
            {"hyperliquidChain", "string"},
            {"agentAddress", "address"},
            {"agentName", "string"},
            {"nonce", "uint64"},
        },
        "HyperliquidTransaction:ApproveAgent", isMainnet);

    if (!agentName) action.erase("agentName");

    body["action"] = action;
    body["nonce"] = nonce;

    nlohmann::ordered_json signatureJson;
    signatureJson["r"] = signature.r;
    signatureJson["s"] = signature.s;
    signatureJson["v"] = signature.v;
    body["signature"] = signatureJson;

    return body;
}

Signature Signing::signL1Action(
    const Wallet& wallet,
    const nlohmann::ordered_json& action,
    const std::optional<std::string>& vaultAddress,
    uint64_t nonce,
    const std::optional<uint64_t>& expiresAfter,
    bool isMainnet)
{
    auto hash = SigningHelpers::actionHash(action, vaultAddress, nonce, expiresAfter);

    std::string source = isMainnet ? "a" : "b";
    auto structHash = SigningHelpers::agentStructHash(source, hash);
    auto domSep = SigningHelpers::domainSeparatorHash();
    auto finalHash = SigningHelpers::eip712Hash(domSep, structHash);

    return SigningHelpers::ecdsaSign(wallet, finalHash);
}

}
