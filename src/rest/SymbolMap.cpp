#include "SymbolMap.h"

namespace hyperliquid {

void SymbolMap::add(const std::string& symbol, int securityId)
{
    symbolToId_.insert({symbol, securityId});
}

int SymbolMap::resolve(const std::string& symbol) const
{
    auto iter = symbolToId_.find(symbol);
    if (iter == symbolToId_.end())
    {
        throw std::invalid_argument("Unknown symbol: " + symbol);
    }
    return iter->second;
}

} // namespace hyperliquid
