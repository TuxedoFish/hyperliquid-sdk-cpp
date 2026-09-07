#pragma once

#include <fstream>
#include <stdexcept>
#include <string>

#include <nlohmann/json.hpp>
#include "hyperliquid/config/Config.h"

struct TestConfig
{
    hyperliquid::Wallet wallet;
    std::optional<std::string> subaccount;
};

inline TestConfig loadTestConfig(const std::string& path = EXAMPLES_DIR "test.json")
{
    std::ifstream file(path);
    if (!file.is_open())
        throw std::runtime_error(
            "Could not open config file: " + path +
            "\nThis file is gitignored and not shipped with the repo - copy examples/example.json "
            "to examples/test.json and fill in a testnet wallet's address/private key (and "
            "optionally a subaccount address) before running examples. See the README's "
            "\"Quickstart\" section, or https://TuxedoFish.github.io/hyperliquid-sdk-cpp/examples/#credentials.");

    auto json = nlohmann::json::parse(file);

    TestConfig config;
    config.wallet = hyperliquid::Wallet{
        json.at("wallet").get<std::string>(),
        json.at("privateKey").get<std::string>()
    };

    if (json.contains("subaccount") && !json["subaccount"].is_null())
        config.subaccount = json["subaccount"].get<std::string>();

    return config;
}

inline hyperliquid::Wallet loadWalletFromConfig(const std::string& path = EXAMPLES_DIR "test.json")
{
    return loadTestConfig(path).wallet;
}
