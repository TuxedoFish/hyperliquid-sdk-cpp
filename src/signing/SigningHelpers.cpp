#include "SigningHelpers.h"
#include "Signing.h"

extern "C" {
#include "sha3.h"
}

#include <cstring>
#include <stdexcept>
#include <sstream>
#include <iomanip>
#include <random>

#include <secp256k1.h>
#include <secp256k1_recovery.h>

namespace hyperliquid {

std::array<uint8_t, 32> SigningHelpers::keccak256(const uint8_t* data, size_t length)
{
    std::array<uint8_t, 32> output;
    sha3_HashBuffer(256, SHA3_FLAGS_KECCAK, data, static_cast<unsigned>(length),
                    output.data(), 32);
    return output;
}

std::array<uint8_t, 32> SigningHelpers::keccak256(const std::vector<uint8_t>& data)
{
    return keccak256(data.data(), data.size());
}

std::vector<uint8_t> SigningHelpers::hexToBytes(const std::string& hex)
{
    auto hexCharToNibble = [](char ch) -> uint8_t {
        if (ch >= '0' && ch <= '9') return ch - '0';
        if (ch >= 'a' && ch <= 'f') return ch - 'a' + 10;
        if (ch >= 'A' && ch <= 'F') return ch - 'A' + 10;
        throw std::invalid_argument(std::string("Invalid hex character: ") + ch);
    };

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

std::string SigningHelpers::toHex(const uint8_t* data, size_t length)
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

std::string SigningHelpers::toHexPadded(const uint8_t* data, size_t length)
{
    std::ostringstream stream;
    stream << "0x";
    for (size_t idx = 0; idx < length; idx++)
        stream << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(data[idx]);
    return stream.str();
}

std::array<uint8_t, 32> SigningHelpers::encodeUint256(uint64_t value)
{
    std::array<uint8_t, 32> result = {};
    for (int idx = 0; idx < 8; idx++)
        result[31 - idx] = static_cast<uint8_t>((value >> (idx * 8)) & 0xFF);
    return result;
}

std::array<uint8_t, 32> SigningHelpers::encodeAddress(const std::vector<uint8_t>& addressBytes)
{
    std::array<uint8_t, 32> result = {};
    if (addressBytes.size() == 20)
        std::memcpy(result.data() + 12, addressBytes.data(), 20);
    return result;
}

std::array<uint8_t, 32> SigningHelpers::actionHash(
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

std::array<uint8_t, 32> SigningHelpers::eip712Hash(
    const std::array<uint8_t, 32>& domainSeparator,
    const std::array<uint8_t, 32>& structHash)
{
    std::vector<uint8_t> encoded;
    encoded.push_back(0x19);
    encoded.push_back(0x01);
    encoded.insert(encoded.end(), domainSeparator.begin(), domainSeparator.end());
    encoded.insert(encoded.end(), structHash.begin(), structHash.end());

    return keccak256(encoded);
}

std::array<uint8_t, 32> SigningHelpers::domainSeparatorHash()
{
    auto typeHash = keccak256(
        reinterpret_cast<const uint8_t*>("EIP712Domain(string name,string version,uint256 chainId,address verifyingContract)"),
        strlen("EIP712Domain(string name,string version,uint256 chainId,address verifyingContract)"));

    auto nameHash = keccak256(reinterpret_cast<const uint8_t*>("Exchange"), strlen("Exchange"));
    auto versionHash = keccak256(reinterpret_cast<const uint8_t*>("1"), strlen("1"));
    auto chainId = encodeUint256(1337);
    auto verifyingContract = encodeAddress(std::vector<uint8_t>(20, 0));

    std::vector<uint8_t> encoded;
    encoded.insert(encoded.end(), typeHash.begin(), typeHash.end());
    encoded.insert(encoded.end(), nameHash.begin(), nameHash.end());
    encoded.insert(encoded.end(), versionHash.begin(), versionHash.end());
    encoded.insert(encoded.end(), chainId.begin(), chainId.end());
    encoded.insert(encoded.end(), verifyingContract.begin(), verifyingContract.end());

    return keccak256(encoded);
}

std::array<uint8_t, 32> SigningHelpers::domainSeparatorHash(
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

std::array<uint8_t, 32> SigningHelpers::agentStructHash(
    const std::string& source,
    const std::array<uint8_t, 32>& connectionId)
{
    auto typeHash = keccak256(
        reinterpret_cast<const uint8_t*>("Agent(string source,bytes32 connectionId)"),
        strlen("Agent(string source,bytes32 connectionId)"));

    auto sourceHash = keccak256(
        reinterpret_cast<const uint8_t*>(source.c_str()), source.size());

    std::vector<uint8_t> encoded;
    encoded.insert(encoded.end(), typeHash.begin(), typeHash.end());
    encoded.insert(encoded.end(), sourceHash.begin(), sourceHash.end());
    encoded.insert(encoded.end(), connectionId.begin(), connectionId.end());

    return keccak256(encoded);
}

std::array<uint8_t, 32> SigningHelpers::userSignedStructHash(
    const std::string& primaryType,
    const std::vector<EIP712Field>& payloadTypes,
    const nlohmann::ordered_json& action)
{
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

Signature SigningHelpers::ecdsaSign(
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

std::string SigningHelpers::generatePrivateKeyHex()
{
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dist;

    secp256k1_context* context = secp256k1_context_create(SECP256K1_CONTEXT_SIGN);
    if (!context)
        throw std::runtime_error("Failed to create secp256k1 context");

    std::array<uint8_t, 32> bytes{};
    do
    {
        for (int word = 0; word < 4; word++)
        {
            uint64_t value = dist(gen);
            for (int idx = 0; idx < 8; idx++)
                bytes[word * 8 + idx] = static_cast<uint8_t>((value >> ((7 - idx) * 8)) & 0xFF);
        }
    } while (!secp256k1_ec_seckey_verify(context, bytes.data()));

    secp256k1_context_destroy(context);
    return toHexPadded(bytes.data(), bytes.size());
}

std::string SigningHelpers::privateKeyToAddress(const std::string& privateKeyHex)
{
    auto privateKeyBytes = hexToBytes(privateKeyHex);
    if (privateKeyBytes.size() != 32)
        throw std::invalid_argument("Private key must be 32 bytes");

    secp256k1_context* context = secp256k1_context_create(SECP256K1_CONTEXT_SIGN);
    if (!context)
        throw std::runtime_error("Failed to create secp256k1 context");

    secp256k1_pubkey pubkey;
    if (!secp256k1_ec_pubkey_create(context, &pubkey, privateKeyBytes.data()))
    {
        secp256k1_context_destroy(context);
        throw std::runtime_error("Failed to derive public key from private key");
    }

    uint8_t serialized[65];
    size_t outputLen = sizeof(serialized);
    secp256k1_ec_pubkey_serialize(context, serialized, &outputLen, &pubkey, SECP256K1_EC_UNCOMPRESSED);
    secp256k1_context_destroy(context);

    // Ethereum address = last 20 bytes of keccak256(uncompressed pubkey without the 0x04 prefix).
    auto hash = keccak256(serialized + 1, 64);
    return toHexPadded(hash.data() + 12, 20);
}

}
