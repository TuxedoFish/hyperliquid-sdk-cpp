#pragma once

#include <cstdint>
#include <optional>
#include <string>

#include <nlohmann/json.hpp>

#include "hyperliquid/types/RequestTypes.h"

namespace hyperliquid {

class InfoRequestBuilder {
public:
    static nlohmann::ordered_json spotMeta();
    static nlohmann::ordered_json meta(const std::optional<std::string>& dex = std::nullopt);
    static nlohmann::ordered_json outcomeMeta();
    static nlohmann::ordered_json perpDexs();

    static nlohmann::ordered_json l2Book(const std::string& coin,
                                         const std::optional<int>& nSigFigs = std::nullopt,
                                         const std::optional<int>& mantissa = std::nullopt);
    static nlohmann::ordered_json candleSnapshot(const std::string& coin,
                                                 const std::string& interval,
                                                 uint64_t startTime,
                                                 uint64_t endTime);
    static nlohmann::ordered_json allMids(const std::optional<std::string>& dex = std::nullopt);
    static nlohmann::ordered_json openOrders(const std::string& user,
                                             const std::optional<std::string>& dex = std::nullopt);
    static nlohmann::ordered_json orderStatus(const std::string& user, const OrderId& oid);
    static nlohmann::ordered_json userFills(const std::string& user,
                                            const std::optional<bool>& aggregateByTime = std::nullopt,
                                            const std::optional<std::string>& dex = std::nullopt);
    static nlohmann::ordered_json userFillsByTime(const std::string& user,
                                                  uint64_t startTime,
                                                  const std::optional<uint64_t>& endTime = std::nullopt,
                                                  const std::optional<bool>& aggregateByTime = std::nullopt,
                                                  const std::optional<std::string>& dex = std::nullopt);
    static nlohmann::ordered_json clearinghouseState(const std::string& user,
                                                     const std::optional<std::string>& dex = std::nullopt);
    static nlohmann::ordered_json userRateLimit(const std::string& user);
    static nlohmann::ordered_json metaAndAssetCtxs(const std::optional<std::string>& dex = std::nullopt);
    static nlohmann::ordered_json spotMetaAndAssetCtxs();
    static nlohmann::ordered_json spotClearinghouseState(const std::string& user,
                                                         const std::optional<std::string>& dex = std::nullopt);
    static nlohmann::ordered_json frontendOpenOrders(const std::string& user,
                                                     const std::optional<std::string>& dex = std::nullopt);
    static nlohmann::ordered_json historicalOrders(const std::string& user);
    static nlohmann::ordered_json userTwapSliceFills(const std::string& user);
    static nlohmann::ordered_json subAccounts(const std::string& user);
    static nlohmann::ordered_json userFees(const std::string& user);
    static nlohmann::ordered_json maxBuilderFee(const std::string& user, const std::string& builder);
    static nlohmann::ordered_json approvedBuilders(const std::string& user);

    static nlohmann::ordered_json delegations(const std::string& user);
    static nlohmann::ordered_json delegatorSummary(const std::string& user);
    static nlohmann::ordered_json delegatorHistory(const std::string& user);
    static nlohmann::ordered_json delegatorRewards(const std::string& user);
};

} // namespace hyperliquid
