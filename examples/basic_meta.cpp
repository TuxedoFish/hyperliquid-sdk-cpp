#include <hyperliquid/rest/RestApi.h>
#include <hyperliquid/rest/RestApiMessageParser.h>
#include <iostream>

int main() {
    hyperliquid::RestApi api(hyperliquid::Environment::Mainnet);
    hyperliquid::RestApiMessageParser parser;

    std::cout << "=== Spot ===" << std::endl;
    auto spotMeta = parser.parseSpotMeta(api.spotMeta());

    std::cout << spotMeta.tokens.size() << " assets:" << std::endl;
    for (const auto& asset : spotMeta.tokens) {
        std::cout << "  " << asset.name
                  << "  szDecimals=" << asset.szDecimals
                  << std::endl;
    }

    auto dexes = parser.parsePerpDexs(api.perpDexs());

    std::cout << "=== Perps ===" << std::endl;
    auto defaultMeta = parser.parseMeta(api.meta());

    std::cout << defaultMeta.universe.size() << " assets:" << std::endl;
    for (const auto& asset : defaultMeta.universe) {
        std::cout << "  " << asset.name
                  << "  szDecimals=" << asset.szDecimals
                  << "  maxLeverage=" << asset.maxLeverage
                  << std::endl;
    }
    std::cout << std::endl;

    std::cout << "Found " << dexes.dexes.size() << " HIP-3 perp dexes:" << std::endl;
    for (const auto& dex : dexes.dexes) {
        std::cout << "  " << dex.name << " (" << dex.fullName << ")"
                  << "  deployer=" << dex.deployer << std::endl;
    }
    std::cout << std::endl;

    for (const auto& dex : dexes.dexes) {
        std::cout << "=== " << dex.name << " ===" << std::endl;
        auto meta = parser.parseMeta(api.meta(dex.name));

        std::cout << meta.universe.size() << " assets:" << std::endl;
        for (const auto& asset : meta.universe) {
            std::cout << "  " << asset.name
                      << "  szDecimals=" << asset.szDecimals
                      << "  maxLeverage=" << asset.maxLeverage
                      << std::endl;
        }
        std::cout << std::endl;
    }

    return 0;
}
