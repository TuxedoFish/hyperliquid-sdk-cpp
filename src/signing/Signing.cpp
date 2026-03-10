#include "Signing.h"

extern "C" {
#include "sha3.h"
}

#include <cstring>
#include <stdexcept>
#include <sstream>
#include <iomanip>

#include <secp256k1.h>
#include <secp256k1_recovery.h>

namespace hyperliquid {

namespace {

std::array<uint8_t, 32> keccak256(const uint8_t* data, size_t length)
{
    std::array<uint8_t, 32> output;
    sha3_HashBuffer(256, SHA3_FLAGS_KECCAK, data, static_cast<unsigned>(length),
                    output.data(), 32);
    return output;
}

std::array<uint8_t, 32> keccak256(const std::vector<uint8_t>& data)
{
    return keccak256(data.data(), data.size());
}

uint8_t hexCharToNibble(char ch)
{
    if (ch >= '0' && ch <= '9') return ch - '0';
    if (ch >= 'a' && ch <= 'f') return ch - 'a' + 10;
    if (ch >= 'A' && ch <= 'F') return ch - 'A' + 10;
    throw std::invalid_argument(std::string("Invalid hex character: ") + ch);
}

std::vector<uint8_t> hexToBytes(const std::string& hex)
{
    std::string clean = hex;
    if (clean.size() >= 2 && clean[0] == '0' && (clean[1] == 'x' || clean[1] == 'X'))
        clean = clean.substr(2);

    if (clean.size() % 2 != 0)
        clean = "0" + clean;

    std::vector<uint8_t> bytes(clean.size() / 2);
    for (size_t idx = 0; idx < bytes.size(); idx++)
        bytes[idx] = (hexCharToNibble(clean[idx * 2]) << 4) | hexCharToNibble(clean[idx * 2 + 1]);

    return bytes;
}

// Encode a uint256 value (left-padded to 32 bytes, big-endian)
std::array<uint8_t, 32> encodeUint256(uint64_t value)
{
    std::array<uint8_t, 32> result = {};
    for (int idx = 0; idx < 8; idx++)
        result[31 - idx] = static_cast<uint8_t>((value >> (idx * 8)) & 0xFF);
    return result;
}

// Encode an address as uint256 (left-padded to 32 bytes)
std::array<uint8_t, 32> encodeAddress(const std::vector<uint8_t>& addressBytes)
{
    std::array<uint8_t, 32> result = {};
    if (addressBytes.size() == 20)
        std::memcpy(result.data() + 12, addressBytes.data(), 20);
    return result;
}

}

std::string Signing::toHex(const uint8_t* data, size_t length)
{
    std::ostringstream stream;
    stream << "0x";
    for (size_t idx = 0; idx < length; idx++)
        stream << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(data[idx]);

    std::string result = stream.str();
    size_t firstNonZero = result.find_first_not_of('0', 2);
    if (firstNonZero == std::string::npos) return "0x0";
    return "0x" + result.substr(firstNonZero);
}

std::string Signing::toHexPadded(const uint8_t* data, size_t length)
{
    std::ostringstream stream;
    stream << "0x";
    for (size_t idx = 0; idx < length; idx++)
        stream << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(data[idx]);
    return stream.str();
}

std::array<uint8_t, 32> Signing::actionHash(
    const nlohmann::ordered_json& action,
    const std::optional<std::string>& vaultAddress,
    uint64_t nonce,
    const std::optional<uint64_t>& expiresAfter)
{
    std::vector<uint8_t> data = nlohmann::ordered_json::to_msgpack(action);
    for (int idx = 7; idx >= 0; idx--)
        data.push_back(static_cast<uint8_t>((nonce >> (idx * 8)) & 0xFF));
    if (!vaultAddress)
    {
        data.push_back(0x00);
    }
    else
    {
        data.push_back(0x01);

        auto addrBytes = hexToBytes(*vaultAddress);
        data.insert(data.end(), addrBytes.begin(), addrBytes.end());
    }

    if (expiresAfter)
    {
        data.push_back(0x00);
        uint64_t expires = *expiresAfter;
        for (int idx = 7; idx >= 0; idx--)
            data.push_back(static_cast<uint8_t>((expires >> (idx * 8)) & 0xFF));
    }

    return keccak256(data);
}

std::array<uint8_t, 32> Signing::domainSeparatorHash()
{
    // EIP712Domain(string name,string version,uint256 chainId,address verifyingContract)
    auto typeHash = keccak256(
        reinterpret_cast<const uint8_t*>("EIP712Domain(string name,string version,uint256 chainId,address verifyingContract)"),
        strlen("EIP712Domain(string name,string version,uint256 chainId,address verifyingContract)"));

    auto nameHash = keccak256(reinterpret_cast<const uint8_t*>("Exchange"), strlen("Exchange"));
    auto versionHash = keccak256(reinterpret_cast<const uint8_t*>("1"), strlen("1"));
    auto chainId = encodeUint256(1337);
    auto verifyingContract = encodeAddress(std::vector<uint8_t>(20, 0)); // address(0)

    // Concatenate: typeHash || nameHash || versionHash || chainId || verifyingContract
    std::vector<uint8_t> encoded;
    encoded.insert(encoded.end(), typeHash.begin(), typeHash.end());
    encoded.insert(encoded.end(), nameHash.begin(), nameHash.end());
    encoded.insert(encoded.end(), versionHash.begin(), versionHash.end());
    encoded.insert(encoded.end(), chainId.begin(), chainId.end());
    encoded.insert(encoded.end(), verifyingContract.begin(), verifyingContract.end());

    return keccak256(encoded);
}

std::array<uint8_t, 32> Signing::agentStructHash(
    const std::string& source,
    const std::array<uint8_t, 32>& connectionId)
{
    // Agent(string source,bytes32 connectionId)
    auto typeHash = keccak256(
        reinterpret_cast<const uint8_t*>("Agent(string source,bytes32 connectionId)"),
        strlen("Agent(string source,bytes32 connectionId)"));

    auto sourceHash = keccak256(
        reinterpret_cast<const uint8_t*>(source.c_str()), source.size());

    // Concatenate: typeHash || keccak256(source) || connectionId
    std::vector<uint8_t> encoded;
    encoded.insert(encoded.end(), typeHash.begin(), typeHash.end());
    encoded.insert(encoded.end(), sourceHash.begin(), sourceHash.end());
    encoded.insert(encoded.end(), connectionId.begin(), connectionId.end());

    return keccak256(encoded);
}

std::array<uint8_t, 32> Signing::eip712Hash(
    const std::array<uint8_t, 32>& domainSeparator,
    const std::array<uint8_t, 32>& structHash)
{
    // "\x19\x01" || domainSeparator || structHash
    std::vector<uint8_t> encoded;
    encoded.push_back(0x19);
    encoded.push_back(0x01);
    encoded.insert(encoded.end(), domainSeparator.begin(), domainSeparator.end());
    encoded.insert(encoded.end(), structHash.begin(), structHash.end());

    return keccak256(encoded);
}

Signature Signing::ecdsaSign(
    const Wallet& wallet,
    const std::array<uint8_t, 32>& messageHash)
{
    auto privateKeyBytes = hexToBytes(wallet.privateKey);
    if (privateKeyBytes.size() != 32)
        throw std::invalid_argument("Private key must be 32 bytes");

    secp256k1_context* context = secp256k1_context_create(SECP256K1_CONTEXT_SIGN);
    if (!context)
        throw std::runtime_error("Failed to create secp256k1 context");

    secp256k1_ecdsa_recoverable_signature signature;
    if (!secp256k1_ecdsa_sign_recoverable(context, &signature,
            messageHash.data(), privateKeyBytes.data(), nullptr, nullptr))
    {
        secp256k1_context_destroy(context);
        throw std::runtime_error("secp256k1 signing failed");
    }

    uint8_t serialized[64];
    int recid;
    secp256k1_ecdsa_recoverable_signature_serialize_compact(context, serialized, &recid, &signature);
    if (recid != 0 && recid != 1)
        throw std::runtime_error("Unexpected recovery id: " + std::to_string(recid));

    secp256k1_context_destroy(context);

    Signature result;
    result.r = toHex(serialized, 32);
    result.s = toHex(serialized + 32, 32);
    result.v = recid + 27;
    return result;
}

std::string Signing::addressFromPrivateKey(const std::string& privateKey)
{
    auto privateKeyBytes = hexToBytes(privateKey);
    if (privateKeyBytes.size() != 32)
        throw std::invalid_argument("Private key must be 32 bytes");

    secp256k1_context* context = secp256k1_context_create(SECP256K1_CONTEXT_SIGN);
    if (!context)
        throw std::runtime_error("Failed to create secp256k1 context");

    secp256k1_pubkey pubkey;
    if (!secp256k1_ec_pubkey_create(context, &pubkey, privateKeyBytes.data()))
    {
        secp256k1_context_destroy(context);
        throw std::runtime_error("Failed to derive public key");
    }

    uint8_t uncompressed[65];
    size_t len = 65;
    secp256k1_ec_pubkey_serialize(context, uncompressed, &len, &pubkey, SECP256K1_EC_UNCOMPRESSED);
    secp256k1_context_destroy(context);

    // keccak256 of the 64-byte public key (skip the 0x04 prefix)
    auto hash = keccak256(uncompressed + 1, 64);

    // Ethereum address is the last 20 bytes
    std::ostringstream stream;
    stream << "0x";
    for (int idx = 12; idx < 32; idx++)
        stream << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(hash[idx]);
    return stream.str();
}

std::array<uint8_t, 32> Signing::domainSeparatorHash(
    const std::string& name, const std::string& version, uint64_t chainId)
{
    auto typeHash = keccak256(
        reinterpret_cast<const uint8_t*>("EIP712Domain(string name,string version,uint256 chainId,address verifyingContract)"),
        strlen("EIP712Domain(string name,string version,uint256 chainId,address verifyingContract)"));

    auto nameHash = keccak256(reinterpret_cast<const uint8_t*>(name.c_str()), name.size());
    auto versionHash = keccak256(reinterpret_cast<const uint8_t*>(version.c_str()), version.size());
    auto chainIdEncoded = encodeUint256(chainId);
    auto verifyingContract = encodeAddress(std::vector<uint8_t>(20, 0));

    std::vector<uint8_t> encoded;
    encoded.insert(encoded.end(), typeHash.begin(), typeHash.end());
    encoded.insert(encoded.end(), nameHash.begin(), nameHash.end());
    encoded.insert(encoded.end(), versionHash.begin(), versionHash.end());
    encoded.insert(encoded.end(), chainIdEncoded.begin(), chainIdEncoded.end());
    encoded.insert(encoded.end(), verifyingContract.begin(), verifyingContract.end());

    return keccak256(encoded);
}

std::array<uint8_t, 32> Signing::userSignedStructHash(
    const std::string& primaryType,
    const std::vector<EIP712Field>& payloadTypes,
    const nlohmann::ordered_json& action)
{
    // Build type string: "PrimaryType(type1 name1,type2 name2,...)"
    std::string typeString = primaryType + "(";
    for (size_t idx = 0; idx < payloadTypes.size(); idx++)
    {
        if (idx > 0) typeString += ",";
        typeString += payloadTypes[idx].type + " " + payloadTypes[idx].name;
    }
    typeString += ")";

    auto typeHash = keccak256(
        reinterpret_cast<const uint8_t*>(typeString.c_str()), typeString.size());

    std::vector<uint8_t> encoded;
    encoded.insert(encoded.end(), typeHash.begin(), typeHash.end());

    // Encode each field according to its EIP-712 type
    for (const auto& field : payloadTypes)
    {
        if (field.type == "string")
        {
            auto val = action[field.name].get<std::string>();
            auto valHash = keccak256(reinterpret_cast<const uint8_t*>(val.c_str()), val.size());
            encoded.insert(encoded.end(), valHash.begin(), valHash.end());
        }
        else if (field.type == "uint64")
        {
            auto val = action[field.name].get<uint64_t>();
            auto valEncoded = encodeUint256(val);
            encoded.insert(encoded.end(), valEncoded.begin(), valEncoded.end());
        }
        else if (field.type == "address")
        {
            auto val = action[field.name].get<std::string>();
            auto addrBytes = hexToBytes(val);
            auto addrEncoded = encodeAddress(addrBytes);
            encoded.insert(encoded.end(), addrEncoded.begin(), addrEncoded.end());
        }
        else if (field.type == "bool")
        {
            auto val = action[field.name].get<bool>();
            auto valEncoded = encodeUint256(val ? 1 : 0);
            encoded.insert(encoded.end(), valEncoded.begin(), valEncoded.end());
        }
    }

    return keccak256(encoded);
}

Signature Signing::signUserSignedAction(
    const Wallet& wallet,
    const nlohmann::ordered_json& action,
    const std::vector<EIP712Field>& payloadTypes,
    const std::string& primaryType,
    bool isMainnet)
{
    // signatureChainId: 0x66eee = 421614
    uint64_t chainId = 0x66eee;

    auto structHash = userSignedStructHash(primaryType, payloadTypes, action);
    auto domainSeparator = domainSeparatorHash("HyperliquidSignTransaction", "1", chainId);
    auto finalHash = eip712Hash(domainSeparator, structHash);

    return ecdsaSign(wallet, finalHash);
}

Signature Signing::signL1Action(
    const Wallet& wallet,
    const nlohmann::ordered_json& action,
    const std::optional<std::string>& vaultAddress,
    uint64_t nonce,
    const std::optional<uint64_t>& expiresAfter,
    bool isMainnet)
{
    auto hash = actionHash(action, vaultAddress, nonce, expiresAfter);

    // Step 2: Construct phantom agent
    std::string source = isMainnet ? "a" : "b";

    // Step 3: Build EIP-712 struct hash for the Agent
    auto structHash = agentStructHash(source, hash);

    // Step 4: Build the domain separator (can be cached but computed fresh for clarity)
    auto domainSeparator = domainSeparatorHash();

    // Step 5: Compute the final EIP-712 hash
    auto finalHash = eip712Hash(domainSeparator, structHash);

    // Step 6: Sign with secp256k1
    return ecdsaSign(wallet, finalHash);
}

} // namespace hyperliquid
