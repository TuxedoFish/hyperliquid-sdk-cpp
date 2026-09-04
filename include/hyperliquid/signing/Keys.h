#pragma once

#include <string>

namespace hyperliquid {

std::string generateAgentPrivateKey();
std::string privateKeyToAddress(const std::string& privateKeyHex);

}
