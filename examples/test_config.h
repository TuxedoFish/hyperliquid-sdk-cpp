#pragma once

#include <fstream>
#include <stdexcept>
#include <string>

#include <nlohmann/json.hpp>
#include <hyperliquid/signing/Wallet.h>

inline hyperliquid::Wallet loadWalletFromConfig(const std::string& path = EXAMPLES_DIR "test.json")
{
    std::ifstream file(path);
    if (!file.is_open())
        throw std::runtime_error("Could not open config file: " + path);

    auto config = nlohmann::json::parse(file);
    return hyperliquid::Wallet{
        config.at("wallet").get<std::string>(),
        config.at("privateKey").get<std::string>()
    };
}
