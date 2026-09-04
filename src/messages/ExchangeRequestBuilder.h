#pragma once

#include <optional>
#include <vector>

#include <nlohmann/json.hpp>

#include "../rest/SymbolMap.h"
#include "hyperliquid/rest/RestApi.h"
#include "hyperliquid/types/RequestTypes.h"

namespace hyperliquid {

class ExchangeRequestBuilder {
public:
    ExchangeRequestBuilder() = default;

    void initializeMapping(const ApiConfig& config, RestApi* api);

    nlohmann::ordered_json placeOrder(const std::vector<OrderRequest>& orders,
                              Grouping grouping,
                              const std::optional<Builder>& builder = std::nullopt) const;

    nlohmann::ordered_json cancelOrder(const std::vector<CancelRequest>& cancels) const;

    nlohmann::ordered_json cancelOrderByCloid(const std::vector<CancelByCloidRequest>& cancels) const;

    nlohmann::ordered_json scheduleCancel(const std::optional<uint64_t>& time = std::nullopt) const;

    nlohmann::ordered_json modifyOrder(const ModifyRequest& modify) const;

    nlohmann::ordered_json batchModifyOrder(const std::vector<ModifyRequest>& modifies) const;

    nlohmann::ordered_json updateLeverage(const UpdateLeverageRequest& request) const;

    nlohmann::ordered_json updateIsolatedMargin(const UpdateIsolatedMarginRequest& request) const;

    nlohmann::ordered_json approveAgent(const ApproveAgentRequest& request) const;

private:
    nlohmann::ordered_json buildOrderWire(const OrderRequest& order) const;
    SymbolMap symbolMap_;
};

}
