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

// Thrown by the sync RestApi methods when the HTTP transport itself fails
// (DNS/connect/TLS/write/read errors). It is NOT thrown for application-level
// errors reported by the exchange (status == "err") -- those are surfaced via
// the `status`/`error` fields on the typed response structs so callers can
// inspect them without a try/catch, consistent with how RestApiMessageParser
// already represents them.
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
    PlaceOrderResponse placeOrder(const std::vector<OrderRequest>& orders,
                           Grouping grouping,
                           const std::optional<Builder>& builder = std::nullopt,
                           const std::optional<std::string>& vaultAddress = std::nullopt);
    CancelOrderResponse cancelOrder(const std::vector<CancelRequest>& cancels,
                            const std::optional<std::string>& vaultAddress = std::nullopt);
    CancelOrderResponse cancelOrderByCloid(const std::vector<CancelByCloidRequest>& cancels,
                                   const std::optional<std::string>& vaultAddress = std::nullopt);
    ModifyOrderResponse modifyOrder(const ModifyRequest& modify,
                            const std::optional<std::string>& vaultAddress = std::nullopt);
    ModifyOrderResponse batchModifyOrder(const std::vector<ModifyRequest>& modifies,
                                 const std::optional<std::string>& vaultAddress = std::nullopt);

    void spotMetaAsync();
    void metaAsync(const std::optional<std::string>& dex = std::nullopt);
    void outcomeMetaAsync();
    void perpDexsAsync();
    void placeOrderAsync(const std::vector<OrderRequest>& orders,
                         Grouping grouping,
                         const std::optional<Builder>& builder = std::nullopt,
                         const std::optional<std::string>& vaultAddress = std::nullopt);
    void cancelOrderAsync(const std::vector<CancelRequest>& cancels,
                          const std::optional<std::string>& vaultAddress = std::nullopt);
    void cancelOrderByCloidAsync(const std::vector<CancelByCloidRequest>& cancels,
                                 const std::optional<std::string>& vaultAddress = std::nullopt);
    void modifyOrderAsync(const ModifyRequest& modify,
                          const std::optional<std::string>& vaultAddress = std::nullopt);
    void batchModifyOrderAsync(const std::vector<ModifyRequest>& modifies,
                               const std::optional<std::string>& vaultAddress = std::nullopt);

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

}
