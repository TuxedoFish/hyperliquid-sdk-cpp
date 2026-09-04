#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "../types/RequestTypes.h"
#include "RestApiListener.h"
#include "hyperliquid/config/Config.h"

namespace hyperliquid {

class RestApi {
public:
    explicit RestApi(const ApiConfig& config);
    RestApi(const ApiConfig& config, RestApiListener& listener);
    ~RestApi();

    RestApi(const RestApi&) = delete;
    RestApi& operator=(const RestApi&) = delete;

    std::string spotMeta();
    std::string meta(const std::optional<std::string>& dex = std::nullopt);
    std::string outcomeMeta();
    std::string perpDexs();
    std::string placeOrder(const std::vector<OrderRequest>& orders,
                           Grouping grouping,
                           const std::optional<Builder>& builder = std::nullopt,
                           const std::optional<std::string>& vaultAddress = std::nullopt);
    std::string cancelOrder(const std::vector<CancelRequest>& cancels,
                            const std::optional<std::string>& vaultAddress = std::nullopt);
    std::string cancelOrderByCloid(const std::vector<CancelByCloidRequest>& cancels,
                                   const std::optional<std::string>& vaultAddress = std::nullopt);
    std::string modifyOrder(const ModifyRequest& modify,
                            const std::optional<std::string>& vaultAddress = std::nullopt);
    std::string batchModifyOrder(const std::vector<ModifyRequest>& modifies,
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
