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

    nlohmann::ordered_json cancelOrder(const std::vector<CancelRequest>& cancels) const;

    nlohmann::ordered_json cancelOrderByCloid(const std::vector<CancelByCloidRequest>& cancels) const;

    nlohmann::ordered_json modifyOrder(const ModifyRequest& modify) const;

    nlohmann::ordered_json batchModifyOrder(const std::vector<ModifyRequest>& modifies) const;

private:
    nlohmann::ordered_json buildOrderWire(const OrderRequest& order) const;
    const SymbolMap& symbolMap_;
};

}
