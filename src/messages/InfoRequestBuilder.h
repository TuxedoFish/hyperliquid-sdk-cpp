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
};

} // namespace hyperliquid
