#pragma once

#include <stdexcept>
#include <string>
#include <unordered_map>

namespace hyperliquid {

class SymbolMap {
public:
    void add(const std::string& symbol, int securityId);
    int resolve(const std::string& symbol) const;

private:
    std::unordered_map<std::string, int> symbolToId_;
};

}
