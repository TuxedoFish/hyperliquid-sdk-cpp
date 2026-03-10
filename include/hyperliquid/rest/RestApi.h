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

class RestApi {
public:
    RestApi(Environment env, RestApiListener& listener, Wallet wallet,
           const std::set<std::string>& dexes = {});
    RestApi(Environment env, Wallet wallet,
           const std::set<std::string>& dexes = {});
    RestApi(Environment env, RestApiListener& listener,
           const std::set<std::string>& dexes = {});
    RestApi(Environment env,
           const std::set<std::string>& dexes = {});
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
