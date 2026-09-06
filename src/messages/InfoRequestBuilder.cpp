#include "../messages/InfoRequestBuilder.h"

namespace hyperliquid {

nlohmann::ordered_json InfoRequestBuilder::spotMeta()
{
    nlohmann::ordered_json body;
    body["type"] = toString(RestEndpointType::SpotMeta);
    return body;
}

nlohmann::ordered_json InfoRequestBuilder::meta(const std::optional<std::string>& dex)
{
    nlohmann::ordered_json body;
    body["type"] = toString(RestEndpointType::Meta);
    if (dex) body["dex"] = *dex;
    return body;
}

nlohmann::ordered_json InfoRequestBuilder::outcomeMeta()
{
    nlohmann::ordered_json body;
    body["type"] = toString(RestEndpointType::OutcomeMeta);
    return body;
}

nlohmann::ordered_json InfoRequestBuilder::settledOutcome(int outcome)
{
    nlohmann::ordered_json body;
    body["type"] = toString(RestEndpointType::SettledOutcome);
    body["outcome"] = outcome;
    return body;
}

nlohmann::ordered_json InfoRequestBuilder::perpDexs()
{
    nlohmann::ordered_json body;
    body["type"] = toString(RestEndpointType::PerpDexs);
    return body;
}

nlohmann::ordered_json InfoRequestBuilder::perpsAtOpenInterestCap(const std::optional<std::string>& dex)
{
    nlohmann::ordered_json body;
    body["type"] = toString(RestEndpointType::PerpsAtOpenInterestCap);
    if (dex) body["dex"] = *dex;
    return body;
}

nlohmann::ordered_json InfoRequestBuilder::predictedFundings()
{
    nlohmann::ordered_json body;
    body["type"] = toString(RestEndpointType::PredictedFundings);
    return body;
}

nlohmann::ordered_json InfoRequestBuilder::perpAnnotation(const std::string& coin)
{
    nlohmann::ordered_json body;
    body["type"] = toString(RestEndpointType::PerpAnnotation);
    body["coin"] = coin;
    return body;
}

nlohmann::ordered_json InfoRequestBuilder::perpCategories()
{
    nlohmann::ordered_json body;
    body["type"] = toString(RestEndpointType::PerpCategories);
    return body;
}

nlohmann::ordered_json InfoRequestBuilder::perpConciseAnnotations()
{
    nlohmann::ordered_json body;
    body["type"] = toString(RestEndpointType::PerpConciseAnnotations);
    return body;
}

nlohmann::ordered_json InfoRequestBuilder::allPerpMetas()
{
    nlohmann::ordered_json body;
    body["type"] = toString(RestEndpointType::AllPerpMetas);
    return body;
}

nlohmann::ordered_json InfoRequestBuilder::perpDexLimits(const std::string& dex)
{
    nlohmann::ordered_json body;
    body["type"] = toString(RestEndpointType::PerpDexLimits);
    body["dex"] = dex;
    return body;
}

nlohmann::ordered_json InfoRequestBuilder::perpDexStatus(const std::string& dex)
{
    nlohmann::ordered_json body;
    body["type"] = toString(RestEndpointType::PerpDexStatus);
    body["dex"] = dex;
    return body;
}

nlohmann::ordered_json InfoRequestBuilder::perpDeployAuctionStatus()
{
    nlohmann::ordered_json body;
    body["type"] = toString(RestEndpointType::PerpDeployAuctionStatus);
    return body;
}

nlohmann::ordered_json InfoRequestBuilder::userFunding(const std::string& user,
                                                       uint64_t startTime,
                                                       const std::optional<uint64_t>& endTime)
{
    nlohmann::ordered_json body;
    body["type"] = toString(RestEndpointType::UserFunding);
    body["user"] = user;
    body["startTime"] = startTime;
    if (endTime) body["endTime"] = *endTime;
    return body;
}

nlohmann::ordered_json InfoRequestBuilder::userNonFundingLedgerUpdates(const std::string& user,
                                                                       uint64_t startTime,
                                                                       const std::optional<uint64_t>& endTime)
{
    nlohmann::ordered_json body;
    body["type"] = toString(RestEndpointType::UserNonFundingLedgerUpdates);
    body["user"] = user;
    body["startTime"] = startTime;
    if (endTime) body["endTime"] = *endTime;
    return body;
}

nlohmann::ordered_json InfoRequestBuilder::fundingHistory(const std::string& coin,
                                                          uint64_t startTime,
                                                          const std::optional<uint64_t>& endTime)
{
    nlohmann::ordered_json body;
    body["type"] = toString(RestEndpointType::FundingHistory);
    body["coin"] = coin;
    body["startTime"] = startTime;
    if (endTime) body["endTime"] = *endTime;
    return body;
}

nlohmann::ordered_json InfoRequestBuilder::l2Book(const std::string& coin,
                                                  const std::optional<int>& nSigFigs,
                                                  const std::optional<int>& mantissa)
{
    nlohmann::ordered_json body;
    body["type"] = toString(RestEndpointType::L2Book);
    body["coin"] = coin;
    if (nSigFigs) body["nSigFigs"] = *nSigFigs;
    if (mantissa) body["mantissa"] = *mantissa;
    return body;
}

nlohmann::ordered_json InfoRequestBuilder::candleSnapshot(const std::string& coin,
                                                          const std::string& interval,
                                                          uint64_t startTime,
                                                          uint64_t endTime)
{
    nlohmann::ordered_json req;
    req["coin"] = coin;
    req["interval"] = interval;
    req["startTime"] = startTime;
    req["endTime"] = endTime;

    nlohmann::ordered_json body;
    body["type"] = toString(RestEndpointType::CandleSnapshot);
    body["req"] = req;
    return body;
}

nlohmann::ordered_json InfoRequestBuilder::allMids(const std::optional<std::string>& dex)
{
    nlohmann::ordered_json body;
    body["type"] = toString(RestEndpointType::AllMids);
    if (dex) body["dex"] = *dex;
    return body;
}

nlohmann::ordered_json InfoRequestBuilder::openOrders(const std::string& user,
                                                      const std::optional<std::string>& dex)
{
    nlohmann::ordered_json body;
    body["type"] = toString(RestEndpointType::OpenOrders);
    body["user"] = user;
    if (dex) body["dex"] = *dex;
    return body;
}

nlohmann::ordered_json InfoRequestBuilder::orderStatus(const std::string& user, const OrderId& oid)
{
    nlohmann::ordered_json body;
    body["type"] = toString(RestEndpointType::OrderStatus);
    body["user"] = user;
    std::visit([&body](const auto& value) { body["oid"] = value; }, oid);
    return body;
}

nlohmann::ordered_json InfoRequestBuilder::userFills(const std::string& user,
                                                     const std::optional<bool>& aggregateByTime,
                                                     const std::optional<std::string>& dex)
{
    nlohmann::ordered_json body;
    body["type"] = toString(RestEndpointType::UserFills);
    body["user"] = user;
    if (aggregateByTime) body["aggregateByTime"] = *aggregateByTime;
    if (dex) body["dex"] = *dex;
    return body;
}

nlohmann::ordered_json InfoRequestBuilder::userFillsByTime(const std::string& user,
                                                           uint64_t startTime,
                                                           const std::optional<uint64_t>& endTime,
                                                           const std::optional<bool>& aggregateByTime,
                                                           const std::optional<std::string>& dex)
{
    nlohmann::ordered_json body;
    body["type"] = toString(RestEndpointType::UserFillsByTime);
    body["user"] = user;
    body["startTime"] = startTime;
    if (endTime) body["endTime"] = *endTime;
    if (aggregateByTime) body["aggregateByTime"] = *aggregateByTime;
    if (dex) body["dex"] = *dex;
    return body;
}

nlohmann::ordered_json InfoRequestBuilder::clearinghouseState(const std::string& user,
                                                              const std::optional<std::string>& dex)
{
    nlohmann::ordered_json body;
    body["type"] = toString(RestEndpointType::ClearinghouseState);
    body["user"] = user;
    if (dex) body["dex"] = *dex;
    return body;
}

nlohmann::ordered_json InfoRequestBuilder::userRateLimit(const std::string& user)
{
    nlohmann::ordered_json body;
    body["type"] = toString(RestEndpointType::UserRateLimit);
    body["user"] = user;
    return body;
}

nlohmann::ordered_json InfoRequestBuilder::borrowLendUserState(const std::string& user)
{
    nlohmann::ordered_json body;
    body["type"] = toString(RestEndpointType::BorrowLendUserState);
    body["user"] = user;
    return body;
}

nlohmann::ordered_json InfoRequestBuilder::metaAndAssetCtxs(const std::optional<std::string>& dex)
{
    nlohmann::ordered_json body;
    body["type"] = toString(RestEndpointType::MetaAndAssetCtxs);
    if (dex) body["dex"] = *dex;
    return body;
}

nlohmann::ordered_json InfoRequestBuilder::spotMetaAndAssetCtxs()
{
    nlohmann::ordered_json body;
    body["type"] = toString(RestEndpointType::SpotMetaAndAssetCtxs);
    return body;
}

nlohmann::ordered_json InfoRequestBuilder::spotClearinghouseState(const std::string& user,
                                                                  const std::optional<std::string>& dex)
{
    nlohmann::ordered_json body;
    body["type"] = toString(RestEndpointType::SpotClearinghouseState);
    body["user"] = user;
    if (dex) body["dex"] = *dex;
    return body;
}

nlohmann::ordered_json InfoRequestBuilder::spotDeployState(const std::string& user)
{
    nlohmann::ordered_json body;
    body["type"] = toString(RestEndpointType::SpotDeployState);
    body["user"] = user;
    return body;
}

nlohmann::ordered_json InfoRequestBuilder::spotPairDeployAuctionStatus()
{
    nlohmann::ordered_json body;
    body["type"] = toString(RestEndpointType::SpotPairDeployAuctionStatus);
    return body;
}

nlohmann::ordered_json InfoRequestBuilder::frontendOpenOrders(const std::string& user,
                                                              const std::optional<std::string>& dex)
{
    nlohmann::ordered_json body;
    body["type"] = toString(RestEndpointType::FrontendOpenOrders);
    body["user"] = user;
    if (dex) body["dex"] = *dex;
    return body;
}

nlohmann::ordered_json InfoRequestBuilder::historicalOrders(const std::string& user)
{
    nlohmann::ordered_json body;
    body["type"] = toString(RestEndpointType::HistoricalOrders);
    body["user"] = user;
    return body;
}

nlohmann::ordered_json InfoRequestBuilder::userTwapSliceFills(const std::string& user)
{
    nlohmann::ordered_json body;
    body["type"] = toString(RestEndpointType::UserTwapSliceFills);
    body["user"] = user;
    return body;
}

nlohmann::ordered_json InfoRequestBuilder::subAccounts(const std::string& user)
{
    nlohmann::ordered_json body;
    body["type"] = toString(RestEndpointType::SubAccounts);
    body["user"] = user;
    return body;
}

nlohmann::ordered_json InfoRequestBuilder::userFees(const std::string& user)
{
    nlohmann::ordered_json body;
    body["type"] = toString(RestEndpointType::UserFees);
    body["user"] = user;
    return body;
}

nlohmann::ordered_json InfoRequestBuilder::maxBuilderFee(const std::string& user, const std::string& builder)
{
    nlohmann::ordered_json body;
    body["type"] = toString(RestEndpointType::MaxBuilderFee);
    body["user"] = user;
    body["builder"] = builder;
    return body;
}

nlohmann::ordered_json InfoRequestBuilder::approvedBuilders(const std::string& user)
{
    nlohmann::ordered_json body;
    body["type"] = toString(RestEndpointType::ApprovedBuilders);
    body["user"] = user;
    return body;
}

nlohmann::ordered_json InfoRequestBuilder::delegations(const std::string& user)
{
    nlohmann::ordered_json body;
    body["type"] = toString(RestEndpointType::Delegations);
    body["user"] = user;
    return body;
}

nlohmann::ordered_json InfoRequestBuilder::delegatorSummary(const std::string& user)
{
    nlohmann::ordered_json body;
    body["type"] = toString(RestEndpointType::DelegatorSummary);
    body["user"] = user;
    return body;
}

nlohmann::ordered_json InfoRequestBuilder::delegatorHistory(const std::string& user)
{
    nlohmann::ordered_json body;
    body["type"] = toString(RestEndpointType::DelegatorHistory);
    body["user"] = user;
    return body;
}

nlohmann::ordered_json InfoRequestBuilder::delegatorRewards(const std::string& user)
{
    nlohmann::ordered_json body;
    body["type"] = toString(RestEndpointType::DelegatorRewards);
    body["user"] = user;
    return body;
}


nlohmann::ordered_json InfoRequestBuilder::vaultDetails(const std::string& vaultAddress,
                                                        const std::optional<std::string>& user)
{
    nlohmann::ordered_json body;
    body["type"] = toString(RestEndpointType::VaultDetails);
    body["vaultAddress"] = vaultAddress;
    if (user) body["user"] = *user;
    return body;
}

nlohmann::ordered_json InfoRequestBuilder::userVaultEquities(const std::string& user)
{
    nlohmann::ordered_json body;
    body["type"] = toString(RestEndpointType::UserVaultEquities);
    body["user"] = user;
    return body;
}

nlohmann::ordered_json InfoRequestBuilder::portfolio(const std::string& user)
{
    nlohmann::ordered_json body;
    body["type"] = toString(RestEndpointType::Portfolio);
    body["user"] = user;
    return body;
}

nlohmann::ordered_json InfoRequestBuilder::referral(const std::string& user)
{
    nlohmann::ordered_json body;
    body["type"] = toString(RestEndpointType::Referral);
    body["user"] = user;
    return body;
}

nlohmann::ordered_json InfoRequestBuilder::userRole(const std::string& user)
{
    nlohmann::ordered_json body;
    body["type"] = toString(RestEndpointType::UserRole);
    body["user"] = user;
    return body;
}

nlohmann::ordered_json InfoRequestBuilder::borrowLendReserveState(int token)
{
    nlohmann::ordered_json body;
    body["type"] = toString(RestEndpointType::BorrowLendReserveState);
    body["token"] = token;
    return body;
}

nlohmann::ordered_json InfoRequestBuilder::allBorrowLendReserveStates()
{
    nlohmann::ordered_json body;
    body["type"] = toString(RestEndpointType::AllBorrowLendReserveStates);
    return body;
}

}
