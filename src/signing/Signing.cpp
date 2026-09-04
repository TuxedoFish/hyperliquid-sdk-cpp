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
