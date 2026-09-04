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
    PlaceOrderResponse placeOrder(const std::vector<OrderRequest>& orders,
                           Grouping grouping,
                           const std::optional<Builder>& builder = std::nullopt,
                           const std::optional<std::string>& vaultAddress = std::nullopt);
    CancelOrderResponse cancelOrder(const std::vector<CancelRequest>& cancels,
                            const std::optional<std::string>& vaultAddress = std::nullopt);
    CancelOrderResponse cancelOrderByCloid(const std::vector<CancelByCloidRequest>& cancels,
                                   const std::optional<std::string>& vaultAddress = std::nullopt);
    // Schedules an exchange-side cancel-all ("dead man's switch"). Passing no time (or std::nullopt)
    // disarms any previously scheduled cancel; a future unix-ms timestamp arms it.
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

    void spotMetaAsync();
    void metaAsync(const std::optional<std::string>& dex = std::nullopt);
    void outcomeMetaAsync();
    void perpDexsAsync();
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

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

}
