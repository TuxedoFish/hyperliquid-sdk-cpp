#pragma once

#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

#include "../types/RequestTypes.h"
#include "../types/ResponseTypes.h"
#include "RestApiListener.h"
#include "hyperliquid/config/Config.h"

namespace hyperliquid {

// Thrown by the sync RestApi methods on HTTP transport failure only; application-level "err" responses are returned as typed values, not thrown.
class RestApiTransportError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

class RestApiRateLimitError : public RestApiTransportError {
public:
    using RestApiTransportError::RestApiTransportError;
};

class RestApi {
public:
    explicit RestApi(const ApiConfig& config);
    RestApi(const ApiConfig& config, RestApiListener& listener);
    ~RestApi();

    RestApi(const RestApi&) = delete;
    RestApi& operator=(const RestApi&) = delete;

    SpotMetaResponse spotMeta();
    MetaResponse meta(const std::optional<std::string>& dex = std::nullopt);
    OutcomeMetaResponse outcomeMeta();
    PerpDexsResponse perpDexs();
    PerpsAtOpenInterestCapResponse perpsAtOpenInterestCap(const std::optional<std::string>& dex = std::nullopt);
    PredictedFundingsResponse predictedFundings();
    PerpAnnotationResponse perpAnnotation(const std::string& coin);
    PerpCategoriesResponse perpCategories();
    PerpConciseAnnotationsResponse perpConciseAnnotations();
    AllPerpMetasResponse allPerpMetas();
    L2BookResponse l2Book(const std::string& coin,
                          const std::optional<int>& nSigFigs = std::nullopt,
                          const std::optional<int>& mantissa = std::nullopt);
    CandleSnapshotResponse candleSnapshot(const std::string& coin,
                                          const std::string& interval,
                                          uint64_t startTime,
                                          uint64_t endTime);
    AllMidsResponse allMids(const std::optional<std::string>& dex = std::nullopt);
    OpenOrdersResponse openOrders(const std::string& user,
                                  const std::optional<std::string>& dex = std::nullopt);
    OrderStatusResponse orderStatus(const std::string& user, const OrderId& oid);
    UserFillsResponse userFills(const std::string& user,
                                const std::optional<bool>& aggregateByTime = std::nullopt,
                                const std::optional<std::string>& dex = std::nullopt);
    UserFillsResponse userFillsByTime(const std::string& user,
                                      uint64_t startTime,
                                      const std::optional<uint64_t>& endTime = std::nullopt,
                                      const std::optional<bool>& aggregateByTime = std::nullopt,
                                      const std::optional<std::string>& dex = std::nullopt);
    ClearinghouseState clearinghouseState(const std::string& user,
                                          const std::optional<std::string>& dex = std::nullopt);
    UserRateLimitResponse userRateLimit(const std::string& user);
    MetaAndAssetCtxsResponse metaAndAssetCtxs(const std::optional<std::string>& dex = std::nullopt);
    SpotMetaAndAssetCtxsResponse spotMetaAndAssetCtxs();
    SpotClearinghouseStateResponse spotClearinghouseState(const std::string& user,
                                                          const std::optional<std::string>& dex = std::nullopt);
    FrontendOpenOrdersResponse frontendOpenOrders(const std::string& user,
                                                  const std::optional<std::string>& dex = std::nullopt);
    HistoricalOrdersResponse historicalOrders(const std::string& user);
    UserTwapSliceFillsResponse userTwapSliceFills(const std::string& user);
    SubAccountsResponse subAccounts(const std::string& user);
    UserFeesResponse userFees(const std::string& user);
    MaxBuilderFeeResponse maxBuilderFee(const std::string& user, const std::string& builder);
    ApprovedBuildersResponse approvedBuilders(const std::string& user);
    VaultDetailsResponse vaultDetails(const std::string& vaultAddress,
                                      const std::optional<std::string>& user = std::nullopt);
    UserVaultEquitiesResponse userVaultEquities(const std::string& user);
    PortfolioResponse portfolio(const std::string& user);
    ReferralResponse referral(const std::string& user);
    UserRoleResponse userRole(const std::string& user);
    PlaceOrderResponse placeOrder(const std::vector<OrderRequest>& orders,
                           Grouping grouping,
                           const std::optional<Builder>& builder = std::nullopt,
                           const std::optional<std::string>& vaultAddress = std::nullopt);
    CancelOrderResponse cancelOrder(const std::vector<CancelRequest>& cancels,
                            const std::optional<std::string>& vaultAddress = std::nullopt);
    CancelOrderResponse cancelOrderByCloid(const std::vector<CancelByCloidRequest>& cancels,
                                   const std::optional<std::string>& vaultAddress = std::nullopt);
    SimpleResponse scheduleCancel(const std::optional<uint64_t>& time = std::nullopt,
                          const std::optional<std::string>& vaultAddress = std::nullopt);
    ModifyOrderResponse modifyOrder(const ModifyRequest& modify,
                            const std::optional<std::string>& vaultAddress = std::nullopt);
    ModifyOrderResponse batchModifyOrder(const std::vector<ModifyRequest>& modifies,
                                 const std::optional<std::string>& vaultAddress = std::nullopt);
    SimpleResponse updateLeverage(const UpdateLeverageRequest& request,
                          const std::optional<std::string>& vaultAddress = std::nullopt);
    SimpleResponse updateIsolatedMargin(const UpdateIsolatedMarginRequest& request,
                          const std::optional<std::string>& vaultAddress = std::nullopt);
    SimpleResponse approveAgent(const ApproveAgentRequest& request);
    TwapOrderResponse twapOrder(const TwapOrderRequest& request,
                          const std::optional<std::string>& vaultAddress = std::nullopt);
    TwapCancelResponse twapCancel(const TwapCancelRequest& request,
                          const std::optional<std::string>& vaultAddress = std::nullopt);
    // vaultTransfer/usdClassTransfer/sendAsset/usdSend/spotSend/withdraw3/approveBuilderFee move
    // funds against the calling wallet directly (their target vault/destination/etc. is a field
    // of the request itself), so unlike the other exchange methods they do not take a
    // vaultAddress parameter.
    SimpleResponse vaultTransfer(const VaultTransferRequest& request);
    SimpleResponse usdClassTransfer(const UsdClassTransferRequest& request);
    SimpleResponse sendAsset(const SendAssetRequest& request);
    SimpleResponse usdSend(const UsdSendRequest& request);
    SimpleResponse spotSend(const SpotSendRequest& request);
    SimpleResponse withdraw3(const Withdraw3Request& request);
    SimpleResponse approveBuilderFee(const ApproveBuilderFeeRequest& request);
    SimpleResponse cDeposit(uint64_t wei);
    SimpleResponse cWithdraw(uint64_t wei);
    SimpleResponse tokenDelegate(const TokenDelegateRequest& request);
    DelegationsResponse delegations(const std::string& user);
    DelegatorSummaryResponse delegatorSummary(const std::string& user);
    DelegatorHistoryResponse delegatorHistory(const std::string& user);
    DelegatorRewardsResponse delegatorRewards(const std::string& user);
    SimpleResponse sendToEvmWithData(const SendToEvmWithDataRequest& request);
    SimpleResponse agentSendAsset(const AgentSendAssetRequest& request,
                          const std::optional<std::string>& vaultAddress = std::nullopt);
    SimpleResponse reserveRequestWeight(const ReserveRequestWeightRequest& request,
                          const std::optional<std::string>& vaultAddress = std::nullopt);
    SimpleResponse noop(const std::optional<std::string>& vaultAddress = std::nullopt);

    void spotMetaAsync();
    void metaAsync(const std::optional<std::string>& dex = std::nullopt);
    void outcomeMetaAsync();
    void perpDexsAsync();
    void perpsAtOpenInterestCapAsync(const std::optional<std::string>& dex = std::nullopt);
    void predictedFundingsAsync();
    void perpAnnotationAsync(const std::string& coin);
    void perpCategoriesAsync();
    void perpConciseAnnotationsAsync();
    void allPerpMetasAsync();
    void l2BookAsync(const std::string& coin,
                     const std::optional<int>& nSigFigs = std::nullopt,
                     const std::optional<int>& mantissa = std::nullopt);
    void candleSnapshotAsync(const std::string& coin,
                             const std::string& interval,
                             uint64_t startTime,
                             uint64_t endTime);
    void allMidsAsync(const std::optional<std::string>& dex = std::nullopt);
    void openOrdersAsync(const std::string& user,
                         const std::optional<std::string>& dex = std::nullopt);
    void orderStatusAsync(const std::string& user, const OrderId& oid);
    void userFillsAsync(const std::string& user,
                        const std::optional<bool>& aggregateByTime = std::nullopt,
                        const std::optional<std::string>& dex = std::nullopt);
    void userFillsByTimeAsync(const std::string& user,
                              uint64_t startTime,
                              const std::optional<uint64_t>& endTime = std::nullopt,
                              const std::optional<bool>& aggregateByTime = std::nullopt,
                              const std::optional<std::string>& dex = std::nullopt);
    void clearinghouseStateAsync(const std::string& user,
                                 const std::optional<std::string>& dex = std::nullopt);
    void userRateLimitAsync(const std::string& user);
    void metaAndAssetCtxsAsync(const std::optional<std::string>& dex = std::nullopt);
    void spotMetaAndAssetCtxsAsync();
    void spotClearinghouseStateAsync(const std::string& user,
                                     const std::optional<std::string>& dex = std::nullopt);
    void frontendOpenOrdersAsync(const std::string& user,
                                 const std::optional<std::string>& dex = std::nullopt);
    void historicalOrdersAsync(const std::string& user);
    void userTwapSliceFillsAsync(const std::string& user);
    void subAccountsAsync(const std::string& user);
    void userFeesAsync(const std::string& user);
    void maxBuilderFeeAsync(const std::string& user, const std::string& builder);
    void approvedBuildersAsync(const std::string& user);
    void vaultDetailsAsync(const std::string& vaultAddress,
                           const std::optional<std::string>& user = std::nullopt);
    void userVaultEquitiesAsync(const std::string& user);
    void portfolioAsync(const std::string& user);
    void referralAsync(const std::string& user);
    void userRoleAsync(const std::string& user);
    void placeOrderAsync(const std::vector<OrderRequest>& orders,
                         Grouping grouping,
                         const std::optional<Builder>& builder = std::nullopt,
                         const std::optional<std::string>& vaultAddress = std::nullopt);
    void cancelOrderAsync(const std::vector<CancelRequest>& cancels,
                          const std::optional<std::string>& vaultAddress = std::nullopt);
    void cancelOrderByCloidAsync(const std::vector<CancelByCloidRequest>& cancels,
                                 const std::optional<std::string>& vaultAddress = std::nullopt);
    void scheduleCancelAsync(const std::optional<uint64_t>& time = std::nullopt,
                             const std::optional<std::string>& vaultAddress = std::nullopt);
    void modifyOrderAsync(const ModifyRequest& modify,
                          const std::optional<std::string>& vaultAddress = std::nullopt);
    void batchModifyOrderAsync(const std::vector<ModifyRequest>& modifies,
                               const std::optional<std::string>& vaultAddress = std::nullopt);
    void updateLeverageAsync(const UpdateLeverageRequest& request,
                             const std::optional<std::string>& vaultAddress = std::nullopt);
    void updateIsolatedMarginAsync(const UpdateIsolatedMarginRequest& request,
                                   const std::optional<std::string>& vaultAddress = std::nullopt);
    void approveAgentAsync(const ApproveAgentRequest& request);
    void twapOrderAsync(const TwapOrderRequest& request,
                                   const std::optional<std::string>& vaultAddress = std::nullopt);
    void twapCancelAsync(const TwapCancelRequest& request,
                                   const std::optional<std::string>& vaultAddress = std::nullopt);
    void vaultTransferAsync(const VaultTransferRequest& request);
    void usdClassTransferAsync(const UsdClassTransferRequest& request);
    void sendAssetAsync(const SendAssetRequest& request);
    void usdSendAsync(const UsdSendRequest& request);
    void spotSendAsync(const SpotSendRequest& request);
    void withdraw3Async(const Withdraw3Request& request);
    void approveBuilderFeeAsync(const ApproveBuilderFeeRequest& request);
    void cDepositAsync(uint64_t wei);
    void cWithdrawAsync(uint64_t wei);
    void tokenDelegateAsync(const TokenDelegateRequest& request);
    void delegationsAsync(const std::string& user);
    void delegatorSummaryAsync(const std::string& user);
    void delegatorHistoryAsync(const std::string& user);
    void delegatorRewardsAsync(const std::string& user);
    void sendToEvmWithDataAsync(const SendToEvmWithDataRequest& request);
    void agentSendAssetAsync(const AgentSendAssetRequest& request,
                             const std::optional<std::string>& vaultAddress = std::nullopt);
    void reserveRequestWeightAsync(const ReserveRequestWeightRequest& request,
                                   const std::optional<std::string>& vaultAddress = std::nullopt);
    void noopAsync(const std::optional<std::string>& vaultAddress = std::nullopt);

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

}
