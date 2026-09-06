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

    nlohmann::ordered_json agentSetAbstraction(UserAbstractionMode abstraction) const;

    nlohmann::ordered_json twapOrder(const TwapOrderRequest& request) const;

    nlohmann::ordered_json twapCancel(const TwapCancelRequest& request) const;

    nlohmann::ordered_json vaultTransfer(const VaultTransferRequest& request) const;

    nlohmann::ordered_json hip3LiquidatorTransfer(const Hip3LiquidatorTransferRequest& request) const;

    nlohmann::ordered_json perpDeployRegisterAsset2(const PerpDeployRegisterAsset2Request& request) const;

    nlohmann::ordered_json usdClassTransfer(const UsdClassTransferRequest& request) const;

    nlohmann::ordered_json sendAsset(const SendAssetRequest& request) const;

    nlohmann::ordered_json usdSend(const UsdSendRequest& request) const;

    nlohmann::ordered_json spotSend(const SpotSendRequest& request) const;

    nlohmann::ordered_json withdraw3(const Withdraw3Request& request) const;

    nlohmann::ordered_json approveBuilderFee(const ApproveBuilderFeeRequest& request) const;

    nlohmann::ordered_json userSetAbstraction(const UserSetAbstractionRequest& request) const;

    nlohmann::ordered_json cDeposit(uint64_t wei) const;

    nlohmann::ordered_json cWithdraw(uint64_t wei) const;

    nlohmann::ordered_json tokenDelegate(const TokenDelegateRequest& request) const;

    nlohmann::ordered_json sendToEvmWithData(const SendToEvmWithDataRequest& request) const;

    nlohmann::ordered_json userDexAbstraction(const UserDexAbstractionRequest& request) const;

    nlohmann::ordered_json agentSendAsset(const AgentSendAssetRequest& request) const;

    nlohmann::ordered_json reserveRequestWeight(const ReserveRequestWeightRequest& request) const;

    nlohmann::ordered_json noop() const;

private:
    nlohmann::ordered_json buildOrderWire(const OrderRequest& order) const;
    SymbolMap symbolMap_;
};

}
