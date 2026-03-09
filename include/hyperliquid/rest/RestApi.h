#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "../types/RequestTypes.h"
#include "RestApiListener.h"
#include "hyperliquid/signing/Wallet.h"

namespace hyperliquid {

class RestApi {
public:
    RestApi(Environment env, RestApiListener& listener, Wallet wallet);
    RestApi(Environment env, RestApiListener& listener);
    RestApi(Environment env);
    ~RestApi();

    RestApi(const RestApi&) = delete;
    RestApi& operator=(const RestApi&) = delete;

    std::string spotMeta();
    std::string meta(const std::optional<std::string>& dex = std::nullopt);
    std::string perpDexs();
    std::string placeOrder(const std::vector<OrderRequest>& orders,
                           Grouping grouping,
                           const std::optional<Builder>& builder = std::nullopt);

    void spotMetaAsync();
    void metaAsync(const std::optional<std::string>& dex = std::nullopt);
    void perpDexsAsync();
    void placeOrderAsync(const std::vector<OrderRequest>& orders,
                         Grouping grouping,
                         const std::optional<Builder>& builder = std::nullopt);

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace hyperliquid
