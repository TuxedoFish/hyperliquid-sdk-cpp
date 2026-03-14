#pragma once

#include <memory>
#include <optional>
#include <set>
#include <string>
#include <vector>

#include "../types/RequestTypes.h"
#include "RestApiListener.h"
#include "hyperliquid/signing/Wallet.h"

namespace hyperliquid {

struct RestApiConfig
{
    Environment env;
    std::optional<Wallet> wallet;
    std::set<std::string> dexes;
};

class RestApi {
public:
    explicit RestApi(const RestApiConfig& config);
    RestApi(const RestApiConfig& config, RestApiListener& listener);
    ~RestApi();

    RestApi(const RestApi&) = delete;
    RestApi& operator=(const RestApi&) = delete;

    std::string spotMeta();
    std::string meta(const std::optional<std::string>& dex = std::nullopt);
    std::string perpDexs();
    std::string placeOrder(const std::vector<OrderRequest>& orders,
                           Grouping grouping,
                           const std::optional<Builder>& builder = std::nullopt);
    std::string cancelOrder(const std::vector<CancelRequest>& cancels);
    std::string cancelOrderByCloid(const std::vector<CancelByCloidRequest>& cancels);
    std::string modifyOrder(const ModifyRequest& modify);
    std::string batchModifyOrder(const std::vector<ModifyRequest>& modifies);

    void spotMetaAsync();
    void metaAsync(const std::optional<std::string>& dex = std::nullopt);
    void perpDexsAsync();
    void placeOrderAsync(const std::vector<OrderRequest>& orders,
                         Grouping grouping,
                         const std::optional<Builder>& builder = std::nullopt);
    void cancelOrderAsync(const std::vector<CancelRequest>& cancels);
    void cancelOrderByCloidAsync(const std::vector<CancelByCloidRequest>& cancels);
    void modifyOrderAsync(const ModifyRequest& modify);
    void batchModifyOrderAsync(const std::vector<ModifyRequest>& modifies);

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace hyperliquid
