#pragma once

#include <stdexcept>
#include <string>
#include <unordered_map>

namespace hyperliquid {

class SymbolMap {
public:
    // First-add-wins: ignored if symbol is already mapped. Use for derived/convenience
    // aliases where an earlier, more specific claim should not be displaced.
    void add(const std::string& symbol, int securityId);
    // Unconditional overwrite. Use when registering an asset's own real identifier, which
    // should always take precedence over any alias that happened to claim the same string.
    void set(const std::string& symbol, int securityId);
    int resolve(const std::string& symbol) const;

private:
    std::unordered_map<std::string, int> symbolToId_;
};

}
