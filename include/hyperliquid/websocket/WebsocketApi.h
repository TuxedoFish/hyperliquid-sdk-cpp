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

        // examples/ws_perp_info.cpp
        void perpAnnotation(const std::string& coin, std::optional<uint64_t> correlationId = std::nullopt);
        void perpCategories(std::optional<uint64_t> correlationId = std::nullopt);
        void perpConciseAnnotations(std::optional<uint64_t> correlationId = std::nullopt);
        void allPerpMetas(std::optional<uint64_t> correlationId = std::nullopt);
        void perpsAtOpenInterestCap(const std::optional<std::string>& dex = std::nullopt,
                                    std::optional<uint64_t> correlationId = std::nullopt);
        void predictedFundings(std::optional<uint64_t> correlationId = std::nullopt);

        // examples/ws_metadata.cpp
        void metaAndAssetCtxs(const std::optional<std::string>& dex = std::nullopt,
                              std::optional<uint64_t> correlationId = std::nullopt);
        void spotMetaAndAssetCtxs(std::optional<uint64_t> correlationId = std::nullopt);
        void spotClearinghouseState(const std::string& user,
                                    const std::optional<std::string>& dex = std::nullopt,
                                    std::optional<uint64_t> correlationId = std::nullopt);
        void frontendOpenOrders(const std::string& user,
                                const std::optional<std::string>& dex = std::nullopt,
                                std::optional<uint64_t> correlationId = std::nullopt);
        void historicalOrders(const std::string& user, std::optional<uint64_t> correlationId = std::nullopt);
        void userTwapSliceFills(const std::string& user, std::optional<uint64_t> correlationId = std::nullopt);
        void subAccounts(const std::string& user, std::optional<uint64_t> correlationId = std::nullopt);
        void userFees(const std::string& user, std::optional<uint64_t> correlationId = std::nullopt);
        void maxBuilderFee(const std::string& user, const std::string& builder,
                           std::optional<uint64_t> correlationId = std::nullopt);
        void approvedBuilders(const std::string& user, std::optional<uint64_t> correlationId = std::nullopt);

        // examples/ws_info.cpp
        void l2Book(const std::string& coin,
                   const std::optional<int>& nSigFigs = std::nullopt,
                   const std::optional<int>& mantissa = std::nullopt,
                   std::optional<uint64_t> correlationId = std::nullopt);
        void candleSnapshot(const std::string& coin,
                            const std::string& interval,
                            uint64_t startTime,
                            uint64_t endTime,
                            std::optional<uint64_t> correlationId = std::nullopt);
        void allMids(const std::optional<std::string>& dex = std::nullopt,
                    std::optional<uint64_t> correlationId = std::nullopt);
        void openOrders(const std::string& user,
                        const std::optional<std::string>& dex = std::nullopt,
                        std::optional<uint64_t> correlationId = std::nullopt);
        void orderStatus(const std::string& user, const OrderId& oid,
                         std::optional<uint64_t> correlationId = std::nullopt);
        void userFills(const std::string& user,
                       const std::optional<bool>& aggregateByTime = std::nullopt,
                       const std::optional<std::string>& dex = std::nullopt,
                       std::optional<uint64_t> correlationId = std::nullopt);
        void userFillsByTime(const std::string& user,
                             uint64_t startTime,
                             const std::optional<uint64_t>& endTime = std::nullopt,
                             const std::optional<bool>& aggregateByTime = std::nullopt,
                             const std::optional<std::string>& dex = std::nullopt,
                             std::optional<uint64_t> correlationId = std::nullopt);
        void clearinghouseState(const std::string& user,
                                const std::optional<std::string>& dex = std::nullopt,
                                std::optional<uint64_t> correlationId = std::nullopt);

        // examples/ws_vaults.cpp
        void vaultDetails(const std::string& vaultAddress,
                          const std::optional<std::string>& user = std::nullopt,
                          std::optional<uint64_t> correlationId = std::nullopt);
        void userVaultEquities(const std::string& user, std::optional<uint64_t> correlationId = std::nullopt);
        void portfolio(const std::string& user, std::optional<uint64_t> correlationId = std::nullopt);
        void referral(const std::string& user, std::optional<uint64_t> correlationId = std::nullopt);
        void userRole(const std::string& user, std::optional<uint64_t> correlationId = std::nullopt);

        // examples/ws_user_rate_limit.cpp
        void userRateLimit(const std::string& user, std::optional<uint64_t> correlationId = std::nullopt);

        // examples/ws_hip3_deployer.cpp
        void perpDexLimits(const std::string& dex, std::optional<uint64_t> correlationId = std::nullopt);
        void perpDexStatus(const std::string& dex, std::optional<uint64_t> correlationId = std::nullopt);
        void perpDeployAuctionStatus(std::optional<uint64_t> correlationId = std::nullopt);

        // examples/ws_settled_outcome.cpp
        void settledOutcome(int outcome, std::optional<uint64_t> correlationId = std::nullopt);

        // examples/ws_borrow_lend.cpp
        void borrowLendUserState(const std::string& user, std::optional<uint64_t> correlationId = std::nullopt);
        void borrowLendReserveState(int token, std::optional<uint64_t> correlationId = std::nullopt);
        void allBorrowLendReserveStates(std::optional<uint64_t> correlationId = std::nullopt);

        // examples/ws_spot_deploy.cpp
        void spotDeployState(const std::string& user, std::optional<uint64_t> correlationId = std::nullopt);
        void spotPairDeployAuctionStatus(std::optional<uint64_t> correlationId = std::nullopt);

        // examples/ws_staking.cpp (info half; cDeposit/tokenDelegate/cWithdraw below are the exchange half)
        void delegations(const std::string& user, std::optional<uint64_t> correlationId = std::nullopt);
        void delegatorSummary(const std::string& user, std::optional<uint64_t> correlationId = std::nullopt);
        void delegatorHistory(const std::string& user, std::optional<uint64_t> correlationId = std::nullopt);
        void delegatorRewards(const std::string& user, std::optional<uint64_t> correlationId = std::nullopt);

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

        // examples/ws_leverage.cpp
        void updateLeverage(const UpdateLeverageRequest& request,
                            std::optional<uint64_t> correlationId = std::nullopt,
                            const std::optional<std::string>& vaultAddress = std::nullopt);
        void updateIsolatedMargin(const UpdateIsolatedMarginRequest& request,
                                  std::optional<uint64_t> correlationId = std::nullopt,
                                  const std::optional<std::string>& vaultAddress = std::nullopt);

        // examples/ws_approve_agent.cpp
        void approveAgent(const ApproveAgentRequest& request,
                          std::optional<uint64_t> correlationId = std::nullopt);

        // examples/ws_agent_set_abstraction.cpp
        void agentSetAbstraction(UserAbstractionMode abstraction,
                                 std::optional<uint64_t> correlationId = std::nullopt,
                                 const std::optional<std::string>& vaultAddress = std::nullopt);

        // examples/ws_twap.cpp
        void twapOrder(const TwapOrderRequest& request,
                       std::optional<uint64_t> correlationId = std::nullopt,
                       const std::optional<std::string>& vaultAddress = std::nullopt);
        void twapCancel(const TwapCancelRequest& request,
                        std::optional<uint64_t> correlationId = std::nullopt,
                        const std::optional<std::string>& vaultAddress = std::nullopt);

        // examples/ws_vault_transfer.cpp
        // vaultTransfer/hip3LiquidatorTransfer/usdClassTransfer/sendAsset/usdSend/spotSend/
        // withdraw3/approveBuilderFee/userSetAbstraction/cDeposit/cWithdraw/tokenDelegate/
        // sendToEvmWithData/userDexAbstraction move funds or state against the calling wallet
        // directly (their target vault/dex/destination/etc. is a field of the request itself, or
        // they're EIP-712 user-signed actions that Hyperliquid never accepts a vaultAddress for),
        // so unlike the other exchange methods they do not take a vaultAddress parameter - see
        // RestApi.h for the matching rationale, shared across both transports.
        void vaultTransfer(const VaultTransferRequest& request,
                           std::optional<uint64_t> correlationId = std::nullopt);

        // examples/ws_hip3_liquidator_transfer.cpp
        void hip3LiquidatorTransfer(const Hip3LiquidatorTransferRequest& request,
                                    std::optional<uint64_t> correlationId = std::nullopt);

        // examples/ws_transfers.cpp
        void usdClassTransfer(const UsdClassTransferRequest& request,
                              std::optional<uint64_t> correlationId = std::nullopt);
        void sendAsset(const SendAssetRequest& request,
                       std::optional<uint64_t> correlationId = std::nullopt);
        void usdSend(const UsdSendRequest& request,
                    std::optional<uint64_t> correlationId = std::nullopt);
        void spotSend(const SpotSendRequest& request,
                     std::optional<uint64_t> correlationId = std::nullopt);
        void withdraw3(const Withdraw3Request& request,
                       std::optional<uint64_t> correlationId = std::nullopt);

        // examples/ws_approve_builder_fee.cpp
        void approveBuilderFee(const ApproveBuilderFeeRequest& request,
                               std::optional<uint64_t> correlationId = std::nullopt);

        // examples/ws_user_set_abstraction.cpp
        void userSetAbstraction(const UserSetAbstractionRequest& request,
                                std::optional<uint64_t> correlationId = std::nullopt);

        // examples/ws_staking.cpp (exchange half; delegations/delegatorSummary/delegatorHistory/
        // delegatorRewards above are the info half)
        void cDeposit(uint64_t wei,
                     std::optional<uint64_t> correlationId = std::nullopt);
        void cWithdraw(uint64_t wei,
                      std::optional<uint64_t> correlationId = std::nullopt);
        void tokenDelegate(const TokenDelegateRequest& request,
                           std::optional<uint64_t> correlationId = std::nullopt);

        // examples/ws_misc_actions.cpp
        void sendToEvmWithData(const SendToEvmWithDataRequest& request,
                               std::optional<uint64_t> correlationId = std::nullopt);
        // agentSendAsset/reserveRequestWeight/noop retain a vaultAddress parameter (matching
        // RestApi) even though Hyperliquid's real /exchange API and the official TS SDK's request
        // schemas don't define a vaultAddress field for these actions - see the tracking issue
        // referenced from the vaultAddress standardization PR for a possible follow-up to remove it.
        void agentSendAsset(const AgentSendAssetRequest& request,
                            std::optional<uint64_t> correlationId = std::nullopt,
                            const std::optional<std::string>& vaultAddress = std::nullopt);
        void reserveRequestWeight(const ReserveRequestWeightRequest& request,
                                  std::optional<uint64_t> correlationId = std::nullopt,
                                  const std::optional<std::string>& vaultAddress = std::nullopt);
        void noop(std::optional<uint64_t> correlationId = std::nullopt,
                 const std::optional<std::string>& vaultAddress = std::nullopt);

        // examples/ws_user_dex_abstraction.cpp
        void userDexAbstraction(const UserDexAbstractionRequest& request,
                                std::optional<uint64_t> correlationId = std::nullopt);

        void start();
        void stop();

    private:
        struct Impl;
        std::unique_ptr<Impl> impl_;
    };
}
