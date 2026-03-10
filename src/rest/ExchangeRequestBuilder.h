#pragma once

#include <optional>
#include <vector>

#include <nlohmann/json.hpp>

#include "SymbolMap.h"
#include "hyperliquid/types/RequestTypes.h"

namespace hyperliquid {

class ExchangeRequestBuilder {
public:
    explicit ExchangeRequestBuilder(const SymbolMap& symbolMap);

    nlohmann::ordered_json placeOrder(const std::vector<OrderRequest>& orders,
                              Grouping grouping,
                              const std::optional<Builder>& builder = std::nullopt) const;

private:
    const SymbolMap& symbolMap_;
};

}
