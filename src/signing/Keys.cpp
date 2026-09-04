#include "hyperliquid/signing/Keys.h"

#include "SigningHelpers.h"

namespace hyperliquid {

std::string generateAgentPrivateKey()
{
    return SigningHelpers::generatePrivateKeyHex();
}

std::string privateKeyToAddress(const std::string& privateKeyHex)
{
    return SigningHelpers::privateKeyToAddress(privateKeyHex);
}

}
