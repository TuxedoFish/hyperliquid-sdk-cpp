#include "Signing.h"
#include "SigningHelpers.h"

#include <chrono>
#include <spdlog/spdlog.h>

namespace hyperliquid {

namespace {

// EIP-712 field lists for user-signed actions, keyed by RestEndpointType. Field order
// must match the on-chain HyperliquidTransaction:* struct definition.
const std::vector<EIP712Field>& userSignedActionFields(RestEndpointType type)
{
    static const std::vector<EIP712Field> sendToEvmWithDataFields = {
        {"hyperliquidChain", "string"},
        {"token", "string"},
        {"amount", "string"},
        {"sourceDex", "string"},
        {"destinationRecipient", "string"},
        {"addressEncoding", "string"},
        {"destinationChainId", "uint32"},
        {"gasLimit", "uint64"},
        {"data", "bytes"},
        {"nonce", "uint64"},
    };

    switch (type)
    {
    case RestEndpointType::SendToEvmWithData: return sendToEvmWithDataFields;
    default: throw std::invalid_argument("No EIP-712 field list for RestEndpointType: " + toString(type));
    }
}

std::string userSignedActionPrimaryType(RestEndpointType type)
{
    switch (type)
    {
    case RestEndpointType::SendToEvmWithData: return "HyperliquidTransaction:SendToEvmWithData";
    default: throw std::invalid_argument("No EIP-712 primary type for RestEndpointType: " + toString(type));
    }
}

}

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

        Signature signature;
        if (isUserSignedAction(type))
        {
            auto action = body["action"];
            action["nonce"] = nonce;
            action["signatureChainId"] = "0x66eee";
            action["hyperliquidChain"] = isMainnet ? "Mainnet" : "Testnet";
            signature = signUserSignedAction(
                config.wallet.value(), action, userSignedActionFields(type),
                userSignedActionPrimaryType(type), isMainnet);
            body["action"] = action;
        }
        else
        {
            auto action = body["action"];
            signature = signL1Action(
                config.wallet.value(), action, vaultAddress, nonce, expiresAfter, isMainnet);
        }

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
    const std::string& primaryType)
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
        "HyperliquidTransaction:ApproveAgent");

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

nlohmann::ordered_json Signing::prepareUserSignedActionBody(
    const ApiConfig& config,
    RestEndpointType type,
    nlohmann::ordered_json action)
{
    nlohmann::ordered_json body;

    if (!config.wallet.has_value())
    {
        spdlog::error("Wallet not configured, can't send authenticated request: {}", toString(type));
        return body;
    }

    std::string primaryType;
    std::vector<EIP712Field> payloadTypes;
    std::string timeField = "time";

    switch (type)
    {
    case RestEndpointType::UsdSend:
        primaryType = "HyperliquidTransaction:UsdSend";
        payloadTypes = {
            {"hyperliquidChain", "string"}, {"destination", "string"},
            {"amount", "string"}, {"time", "uint64"}};
        break;
    case RestEndpointType::Withdraw3:
        primaryType = "HyperliquidTransaction:Withdraw";
        payloadTypes = {
            {"hyperliquidChain", "string"}, {"destination", "string"},
            {"amount", "string"}, {"time", "uint64"}};
        break;
    case RestEndpointType::SpotSend:
        primaryType = "HyperliquidTransaction:SpotSend";
        payloadTypes = {
            {"hyperliquidChain", "string"}, {"destination", "string"}, {"token", "string"},
            {"amount", "string"}, {"time", "uint64"}};
        break;
    case RestEndpointType::UsdClassTransfer:
        primaryType = "HyperliquidTransaction:UsdClassTransfer";
        payloadTypes = {
            {"hyperliquidChain", "string"}, {"amount", "string"},
            {"toPerp", "bool"}, {"nonce", "uint64"}};
        timeField = "nonce";
        break;
    case RestEndpointType::SendAsset:
        primaryType = "HyperliquidTransaction:SendAsset";
        payloadTypes = {
            {"hyperliquidChain", "string"}, {"destination", "string"}, {"sourceDex", "string"},
            {"destinationDex", "string"}, {"token", "string"}, {"amount", "string"},
            {"fromSubAccount", "string"}, {"nonce", "uint64"}};
        timeField = "nonce";
        break;
    case RestEndpointType::ApproveBuilderFee:
        primaryType = "HyperliquidTransaction:ApproveBuilderFee";
        payloadTypes = {
            {"hyperliquidChain", "string"}, {"maxFeeRate", "string"},
            {"builder", "address"}, {"nonce", "uint64"}};
        timeField = "nonce";
        break;
    case RestEndpointType::CDeposit:
    case RestEndpointType::CWithdraw:
        primaryType = type == RestEndpointType::CDeposit
            ? "HyperliquidTransaction:CDeposit"
            : "HyperliquidTransaction:CWithdraw";
        payloadTypes = {
            {"hyperliquidChain", "string"}, {"wei", "uint64"}, {"nonce", "uint64"}};
        timeField = "nonce";
        break;
    case RestEndpointType::TokenDelegate:
        primaryType = "HyperliquidTransaction:TokenDelegate";
        payloadTypes = {
            {"hyperliquidChain", "string"}, {"validator", "address"}, {"wei", "uint64"},
            {"isUndelegate", "bool"}, {"nonce", "uint64"}};
        timeField = "nonce";
        break;
    default:
        throw std::invalid_argument("Not a user-signed action: " + toString(type));
    }

    uint64_t nonce = static_cast<uint64_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count());
    bool isMainnet = (config.env == Environment::Mainnet);

    action[timeField] = nonce;
    action["signatureChainId"] = "0x66eee";
    action["hyperliquidChain"] = isMainnet ? "Mainnet" : "Testnet";

    auto signature = signUserSignedAction(config.wallet.value(), action, payloadTypes, primaryType);

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
