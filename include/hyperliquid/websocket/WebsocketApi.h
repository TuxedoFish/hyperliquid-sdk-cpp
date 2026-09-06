#pragma once

#include <map>
#include <memory>
#include <string>

#include "WebsocketApiListener.h"
#include "../types/RequestTypes.h"
#include "hyperliquid/config/Config.h"

namespace hyperliquid
{
    class WebsocketApi
    {
    public:
        explicit WebsocketApi(ApiConfig& config, WebsocketApiListener& listener);
        ~WebsocketApi();

        WebsocketApi(const WebsocketApi&) = delete;
        WebsocketApi& operator=(const WebsocketApi&) = delete;

        // Subscription requests
        void subscribe(SubscriptionType type, const std::map<std::string, std::string>& filters = {});
        void unsubscribe(SubscriptionType type, const std::map<std::string, std::string>& filters = {});

        // Post requests over websocket
        void spotMeta(std::optional<uint64_t> correlationId = std::nullopt);
        void meta(const std::optional<std::string>& dex = std::nullopt,
                  std::optional<uint64_t> correlationId = std::nullopt);
        void outcomeMeta(std::optional<uint64_t> correlationId = std::nullopt);
        void perpDexs(std::optional<uint64_t> correlationId = std::nullopt);
        void placeOrder(const std::vector<OrderRequest>& orders,
                               Grouping grouping,
                               const std::optional<Builder>& builder = std::nullopt,
                               std::optional<uint64_t> correlationId = std::nullopt,
                               const std::optional<std::string>& vaultAddress = std::nullopt);
        void cancelOrder(const std::vector<CancelRequest>& cancels,
                         std::optional<uint64_t> correlationId = std::nullopt,
                         const std::optional<std::string>& vaultAddress = std::nullopt);
        void cancelOrderByCloid(const std::vector<CancelByCloidRequest>& cancels,
                                std::optional<uint64_t> correlationId = std::nullopt,
                                const std::optional<std::string>& vaultAddress = std::nullopt);
        // Schedules an exchange-side cancel-all ("dead man's switch"). Passing no time (or std::nullopt)
        // disarms any previously scheduled cancel; a future unix-ms timestamp arms it.
        void scheduleCancel(const std::optional<uint64_t>& time = std::nullopt,
                            std::optional<uint64_t> correlationId = std::nullopt,
                            const std::optional<std::string>& vaultAddress = std::nullopt);
        void modifyOrder(const ModifyRequest& modify,
                         std::optional<uint64_t> correlationId = std::nullopt,
                         const std::optional<std::string>& vaultAddress = std::nullopt);
        void batchModifyOrder(const std::vector<ModifyRequest>& modifies,
                              std::optional<uint64_t> correlationId = std::nullopt,
                              const std::optional<std::string>& vaultAddress = std::nullopt);
        void updateLeverage(const UpdateLeverageRequest& request,
                            std::optional<uint64_t> correlationId = std::nullopt,
                            const std::optional<std::string>& vaultAddress = std::nullopt);
        void updateIsolatedMargin(const UpdateIsolatedMarginRequest& request,
                                  std::optional<uint64_t> correlationId = std::nullopt,
                                  const std::optional<std::string>& vaultAddress = std::nullopt);
        void approveAgent(const ApproveAgentRequest& request,
                          std::optional<uint64_t> correlationId = std::nullopt,
                          const std::optional<std::string>& vaultAddress = std::nullopt);
        void agentSetAbstraction(UserAbstractionMode abstraction,
                                 std::optional<uint64_t> correlationId = std::nullopt,
                                 const std::optional<std::string>& vaultAddress = std::nullopt);
        void twapOrder(const TwapOrderRequest& request,
                       std::optional<uint64_t> correlationId = std::nullopt,
                       const std::optional<std::string>& vaultAddress = std::nullopt);
        void twapCancel(const TwapCancelRequest& request,
                        std::optional<uint64_t> correlationId = std::nullopt,
                        const std::optional<std::string>& vaultAddress = std::nullopt);
        void vaultTransfer(const VaultTransferRequest& request,
                           std::optional<uint64_t> correlationId = std::nullopt,
                           const std::optional<std::string>& vaultAddress = std::nullopt);
        void hip3LiquidatorTransfer(const Hip3LiquidatorTransferRequest& request,
                                    std::optional<uint64_t> correlationId = std::nullopt,
                                    const std::optional<std::string>& vaultAddress = std::nullopt);
        void usdClassTransfer(const UsdClassTransferRequest& request,
                              std::optional<uint64_t> correlationId = std::nullopt,
                              const std::optional<std::string>& vaultAddress = std::nullopt);
        void sendAsset(const SendAssetRequest& request,
                       std::optional<uint64_t> correlationId = std::nullopt,
                       const std::optional<std::string>& vaultAddress = std::nullopt);
        void usdSend(const UsdSendRequest& request,
                    std::optional<uint64_t> correlationId = std::nullopt,
                    const std::optional<std::string>& vaultAddress = std::nullopt);
        void spotSend(const SpotSendRequest& request,
                     std::optional<uint64_t> correlationId = std::nullopt,
                     const std::optional<std::string>& vaultAddress = std::nullopt);
        void withdraw3(const Withdraw3Request& request,
                       std::optional<uint64_t> correlationId = std::nullopt,
                       const std::optional<std::string>& vaultAddress = std::nullopt);
        void approveBuilderFee(const ApproveBuilderFeeRequest& request,
                               std::optional<uint64_t> correlationId = std::nullopt,
                               const std::optional<std::string>& vaultAddress = std::nullopt);
        void userSetAbstraction(const UserSetAbstractionRequest& request,
                                std::optional<uint64_t> correlationId = std::nullopt,
                                const std::optional<std::string>& vaultAddress = std::nullopt);
        void cDeposit(uint64_t wei,
                     std::optional<uint64_t> correlationId = std::nullopt,
                     const std::optional<std::string>& vaultAddress = std::nullopt);
        void cWithdraw(uint64_t wei,
                      std::optional<uint64_t> correlationId = std::nullopt,
                      const std::optional<std::string>& vaultAddress = std::nullopt);
        void tokenDelegate(const TokenDelegateRequest& request,
                           std::optional<uint64_t> correlationId = std::nullopt,
                           const std::optional<std::string>& vaultAddress = std::nullopt);
        void sendToEvmWithData(const SendToEvmWithDataRequest& request,
                               std::optional<uint64_t> correlationId = std::nullopt,
                               const std::optional<std::string>& vaultAddress = std::nullopt);
        void userDexAbstraction(const UserDexAbstractionRequest& request,
                                std::optional<uint64_t> correlationId = std::nullopt,
                                const std::optional<std::string>& vaultAddress = std::nullopt);
        void agentSendAsset(const AgentSendAssetRequest& request,
                            std::optional<uint64_t> correlationId = std::nullopt,
                            const std::optional<std::string>& vaultAddress = std::nullopt);
        void reserveRequestWeight(const ReserveRequestWeightRequest& request,
                                  std::optional<uint64_t> correlationId = std::nullopt,
                                  const std::optional<std::string>& vaultAddress = std::nullopt);
        void noop(std::optional<uint64_t> correlationId = std::nullopt,
                 const std::optional<std::string>& vaultAddress = std::nullopt);

        void start();
        void stop();

    private:
        struct Impl;
        std::unique_ptr<Impl> impl_;
    };
}
