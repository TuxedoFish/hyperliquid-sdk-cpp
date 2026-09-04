#include "hyperliquid/rest/RestApi.h"
#include "hyperliquid/rest/RestApiListener.h"
#include "hyperliquid/rest/RestApiMessageParser.h"
#include "HttpSession.h"
#include "../messages/InfoRequestBuilder.h"
#include "../messages/ExchangeRequestBuilder.h"
#include "SymbolMap.h"
#include "signing/Signing.h"

#include <chrono>
#include <future>
#include <memory>
#include <set>
#include <thread>

#include <boost/asio/executor_work_guard.hpp>

#include <nlohmann/json.hpp>

#include "../config/Logger.h"
#include "hyperliquid/config/Config.h"

namespace hyperliquid {

namespace beast = boost::beast;
namespace net = boost::asio;
namespace ssl = boost::asio::ssl;

static RestApiListener defaultListener;

struct RestApi::Impl {
    net::io_context ioc;
    net::executor_work_guard<net::io_context::executor_type> work;
    ssl::context sslCtx;
    std::thread thread;
    std::string host;
    std::string port;
    RestApiListener& listener;
    const ApiConfig& config;
    ExchangeRequestBuilder exchangeRequestBuilder;

    Impl(const ApiConfig& config, RestApiListener& listener)
        : work(net::make_work_guard(ioc))
        , sslCtx(ssl::context::tlsv12_client)
        , host(toInfoEndpoint(config.env).host)
        , port(toInfoEndpoint(config.env).port)
        , listener(listener)
        , config(config)
    {
        sslCtx.set_default_verify_paths();
        sslCtx.set_verify_mode(ssl::verify_peer);
        thread = std::thread([this]() { ioc.run(); });
    }

    ~Impl()
    {
        work.reset();
        ioc.stop();
        if (thread.joinable()) thread.join();
    }

    // approveAgent is a user-signed action rather than an L1 action (see Signing.h), so it is
    // routed to Signing::prepareApproveAgentBody instead of the generic vaultAddress/expiresAfter
    // path used by the other (L1) exchange endpoints.
    nlohmann::ordered_json prepareBodyForType(RestEndpointType type, nlohmann::ordered_json body,
                                              const std::optional<std::string>& vaultAddress,
                                              const std::optional<uint64_t>& expiresAfter)
    {
        if (type == RestEndpointType::ApproveAgent)
        {
            std::string agentAddress = body["action"].at("agentAddress").get<std::string>();
            std::optional<std::string> agentName;
            if (body["action"].contains("agentName"))
                agentName = body["action"].at("agentName").get<std::string>();
            return Signing::prepareApproveAgentBody(config, agentAddress, agentName);
        }

        auto effectiveVault = vaultAddress ? vaultAddress : config.vaultAddress;
        return Signing::prepareBody(config, type, std::move(body), effectiveVault, expiresAfter);
    }

    void signAndSend(RestEndpointType type, nlohmann::ordered_json body,
                     const std::optional<std::string>& vaultAddress = std::nullopt,
                     const std::optional<uint64_t>& expiresAfter = std::nullopt)
    {
        auto prepared = prepareBodyForType(type, std::move(body), vaultAddress, expiresAfter);
        std::string serialized = prepared.dump();

        auto session = std::make_shared<HttpSession>(
            ioc, sslCtx, host, port, toPath(type),
            [this, type](const std::string& responseBody, unsigned int statusCode, beast::error_code ec) {
                if (ec) {
                    getLogger()->error("RestApi: error for {}: {}", toString(type), ec.message());
                    listener.onError(type, ec.message());
                    return;
                }
                if (statusCode == 429) {
                    getLogger()->warn("RestApi: rate limited (HTTP 429) for {}", toString(type));
                    listener.onRateLimitExceeded(type, responseBody);
                    return;
                }
                listener.onMessage(responseBody, type);
            });

        session->run(serialized);
    }

    std::string signAndSendSync(RestEndpointType type, nlohmann::ordered_json body,
                                const std::optional<std::string>& vaultAddress = std::nullopt,
                                const std::optional<uint64_t>& expiresAfter = std::nullopt)
    {
        auto prepared = prepareBodyForType(type, std::move(body), vaultAddress, expiresAfter);
        std::string serialized = prepared.dump();
        getLogger()->debug("{}", serialized);

        auto promise = std::make_shared<std::promise<std::string>>();
        auto future = promise->get_future();

        auto session = std::make_shared<HttpSession>(
            ioc, sslCtx, host, port, toPath(type),
            [promise](const std::string& responseBody, unsigned int statusCode, beast::error_code ec) {
                if (ec) {
                    promise->set_exception(std::make_exception_ptr(
                        RestApiTransportError("RestApi: " + ec.message())));
                    return;
                }
                if (statusCode == 429) {
                    promise->set_exception(std::make_exception_ptr(
                        RestApiRateLimitError("RestApi: HTTP 429 rate limited: " + responseBody)));
                    return;
                }
                promise->set_value(responseBody);
            });

        session->run(serialized);
        return future.get();
    }
};

RestApi::RestApi(const ApiConfig& config)
    : impl_(std::make_unique<Impl>(config, defaultListener))
{
    if (!config.skipBuildingSymbolMap) {
        impl_->exchangeRequestBuilder.initializeMapping(config, this);
    }
}

RestApi::RestApi(const ApiConfig& config, RestApiListener& listener)
    : impl_(std::make_unique<Impl>(config, listener))
{
    if (!config.skipBuildingSymbolMap) {
        impl_->exchangeRequestBuilder.initializeMapping(config, this);
    }
}

RestApi::~RestApi() = default;

SpotMetaResponse RestApi::spotMeta()
{
    return RestApiMessageParser().parseSpotMeta(
        impl_->signAndSendSync(RestEndpointType::SpotMeta, InfoRequestBuilder::spotMeta()));
}

MetaResponse RestApi::meta(const std::optional<std::string>& dex)
{
    return RestApiMessageParser().parseMeta(
        impl_->signAndSendSync(RestEndpointType::Meta, InfoRequestBuilder::meta(dex)));
}

OutcomeMetaResponse RestApi::outcomeMeta()
{
    return RestApiMessageParser().parseOutcomeMeta(
        impl_->signAndSendSync(RestEndpointType::OutcomeMeta, InfoRequestBuilder::outcomeMeta()));
}

PerpDexsResponse RestApi::perpDexs()
{
    return RestApiMessageParser().parsePerpDexs(
        impl_->signAndSendSync(RestEndpointType::PerpDexs, InfoRequestBuilder::perpDexs()));
}

L2BookResponse RestApi::l2Book(const std::string& coin,
                               const std::optional<int>& nSigFigs,
                               const std::optional<int>& mantissa)
{
    return RestApiMessageParser().parseL2Book(
        impl_->signAndSendSync(RestEndpointType::L2Book, InfoRequestBuilder::l2Book(coin, nSigFigs, mantissa)));
}

CandleSnapshotResponse RestApi::candleSnapshot(const std::string& coin,
                                               const std::string& interval,
                                               uint64_t startTime,
                                               uint64_t endTime)
{
    return RestApiMessageParser().parseCandleSnapshot(
        impl_->signAndSendSync(RestEndpointType::CandleSnapshot,
                               InfoRequestBuilder::candleSnapshot(coin, interval, startTime, endTime)));
}

AllMidsResponse RestApi::allMids(const std::optional<std::string>& dex)
{
    return RestApiMessageParser().parseAllMids(
        impl_->signAndSendSync(RestEndpointType::AllMids, InfoRequestBuilder::allMids(dex)));
}

OpenOrdersResponse RestApi::openOrders(const std::string& user, const std::optional<std::string>& dex)
{
    return RestApiMessageParser().parseOpenOrders(
        impl_->signAndSendSync(RestEndpointType::OpenOrders, InfoRequestBuilder::openOrders(user, dex)));
}

OrderStatusResponse RestApi::orderStatus(const std::string& user, const OrderId& oid)
{
    return RestApiMessageParser().parseOrderStatus(
        impl_->signAndSendSync(RestEndpointType::OrderStatus, InfoRequestBuilder::orderStatus(user, oid)));
}

UserFillsResponse RestApi::userFills(const std::string& user,
                                     const std::optional<bool>& aggregateByTime,
                                     const std::optional<std::string>& dex)
{
    return RestApiMessageParser().parseUserFills(
        impl_->signAndSendSync(RestEndpointType::UserFills,
                               InfoRequestBuilder::userFills(user, aggregateByTime, dex)));
}

UserFillsResponse RestApi::userFillsByTime(const std::string& user,
                                           uint64_t startTime,
                                           const std::optional<uint64_t>& endTime,
                                           const std::optional<bool>& aggregateByTime,
                                           const std::optional<std::string>& dex)
{
    return RestApiMessageParser().parseUserFillsByTime(
        impl_->signAndSendSync(RestEndpointType::UserFillsByTime,
                               InfoRequestBuilder::userFillsByTime(user, startTime, endTime, aggregateByTime, dex)));
}

ClearinghouseState RestApi::clearinghouseState(const std::string& user, const std::optional<std::string>& dex)
{
    return RestApiMessageParser().parseClearinghouseState(
        impl_->signAndSendSync(RestEndpointType::ClearinghouseState,
                               InfoRequestBuilder::clearinghouseState(user, dex)));
}

UserRateLimitResponse RestApi::userRateLimit(const std::string& user)
{
    return RestApiMessageParser().parseUserRateLimit(
        impl_->signAndSendSync(RestEndpointType::UserRateLimit, InfoRequestBuilder::userRateLimit(user)));
}

MetaAndAssetCtxsResponse RestApi::metaAndAssetCtxs(const std::optional<std::string>& dex)
{
    return RestApiMessageParser().parseMetaAndAssetCtxs(
        impl_->signAndSendSync(RestEndpointType::MetaAndAssetCtxs, InfoRequestBuilder::metaAndAssetCtxs(dex)));
}

SpotMetaAndAssetCtxsResponse RestApi::spotMetaAndAssetCtxs()
{
    return RestApiMessageParser().parseSpotMetaAndAssetCtxs(
        impl_->signAndSendSync(RestEndpointType::SpotMetaAndAssetCtxs, InfoRequestBuilder::spotMetaAndAssetCtxs()));
}

SpotClearinghouseStateResponse RestApi::spotClearinghouseState(const std::string& user,
                                                                const std::optional<std::string>& dex)
{
    return RestApiMessageParser().parseSpotClearinghouseState(
        impl_->signAndSendSync(RestEndpointType::SpotClearinghouseState,
                               InfoRequestBuilder::spotClearinghouseState(user, dex)));
}

FrontendOpenOrdersResponse RestApi::frontendOpenOrders(const std::string& user, const std::optional<std::string>& dex)
{
    return RestApiMessageParser().parseFrontendOpenOrders(
        impl_->signAndSendSync(RestEndpointType::FrontendOpenOrders,
                               InfoRequestBuilder::frontendOpenOrders(user, dex)));
}

HistoricalOrdersResponse RestApi::historicalOrders(const std::string& user)
{
    return RestApiMessageParser().parseHistoricalOrders(
        impl_->signAndSendSync(RestEndpointType::HistoricalOrders, InfoRequestBuilder::historicalOrders(user)));
}

UserTwapSliceFillsResponse RestApi::userTwapSliceFills(const std::string& user)
{
    return RestApiMessageParser().parseUserTwapSliceFills(
        impl_->signAndSendSync(RestEndpointType::UserTwapSliceFills, InfoRequestBuilder::userTwapSliceFills(user)));
}

SubAccountsResponse RestApi::subAccounts(const std::string& user)
{
    return RestApiMessageParser().parseSubAccounts(
        impl_->signAndSendSync(RestEndpointType::SubAccounts, InfoRequestBuilder::subAccounts(user)));
}

UserFeesResponse RestApi::userFees(const std::string& user)
{
    return RestApiMessageParser().parseUserFees(
        impl_->signAndSendSync(RestEndpointType::UserFees, InfoRequestBuilder::userFees(user)));
}

MaxBuilderFeeResponse RestApi::maxBuilderFee(const std::string& user, const std::string& builder)
{
    return RestApiMessageParser().parseMaxBuilderFee(
        impl_->signAndSendSync(RestEndpointType::MaxBuilderFee, InfoRequestBuilder::maxBuilderFee(user, builder)));
}

ApprovedBuildersResponse RestApi::approvedBuilders(const std::string& user)
{
    return RestApiMessageParser().parseApprovedBuilders(
        impl_->signAndSendSync(RestEndpointType::ApprovedBuilders, InfoRequestBuilder::approvedBuilders(user)));
}

PlaceOrderResponse RestApi::placeOrder(const std::vector<OrderRequest>& orders,
                                Grouping grouping,
                                const std::optional<Builder>& builder,
                                const std::optional<std::string>& vaultAddress)
{
    return RestApiMessageParser().parsePlaceOrder(
        impl_->signAndSendSync(RestEndpointType::PlaceOrder,
                                  impl_->exchangeRequestBuilder.placeOrder(orders, grouping, builder),
                                  vaultAddress));
}

CancelOrderResponse RestApi::cancelOrder(const std::vector<CancelRequest>& cancels,
                                 const std::optional<std::string>& vaultAddress)
{
    return RestApiMessageParser().parseCancelOrder(
        impl_->signAndSendSync(RestEndpointType::CancelOrder,
                                   impl_->exchangeRequestBuilder.cancelOrder(cancels),
                                   vaultAddress));
}

CancelOrderResponse RestApi::cancelOrderByCloid(const std::vector<CancelByCloidRequest>& cancels,
                                        const std::optional<std::string>& vaultAddress)
{
    return RestApiMessageParser().parseCancelOrder(
        impl_->signAndSendSync(RestEndpointType::CancelOrderByCloid,
                                   impl_->exchangeRequestBuilder.cancelOrderByCloid(cancels),
                                   vaultAddress));
}

SimpleResponse RestApi::scheduleCancel(const std::optional<uint64_t>& time,
                               const std::optional<std::string>& vaultAddress)
{
    return RestApiMessageParser().parseSimpleResponse(
        impl_->signAndSendSync(RestEndpointType::ScheduleCancel,
                                   impl_->exchangeRequestBuilder.scheduleCancel(time),
                                   vaultAddress));
}

ModifyOrderResponse RestApi::modifyOrder(const ModifyRequest& modify,
                                 const std::optional<std::string>& vaultAddress)
{
    return RestApiMessageParser().parseModifyOrder(
        impl_->signAndSendSync(RestEndpointType::ModifyOrder,
                                   impl_->exchangeRequestBuilder.modifyOrder(modify),
                                   vaultAddress));
}

ModifyOrderResponse RestApi::batchModifyOrder(const std::vector<ModifyRequest>& modifies,
                                      const std::optional<std::string>& vaultAddress)
{
    return RestApiMessageParser().parseModifyOrder(
        impl_->signAndSendSync(RestEndpointType::BatchModifyOrder,
                                   impl_->exchangeRequestBuilder.batchModifyOrder(modifies),
                                   vaultAddress));
}

SimpleResponse RestApi::updateLeverage(const UpdateLeverageRequest& request,
                               const std::optional<std::string>& vaultAddress)
{
    return RestApiMessageParser().parseSimpleResponse(
        impl_->signAndSendSync(RestEndpointType::UpdateLeverage,
                                   impl_->exchangeRequestBuilder.updateLeverage(request),
                                   vaultAddress));
}

SimpleResponse RestApi::updateIsolatedMargin(const UpdateIsolatedMarginRequest& request,
                               const std::optional<std::string>& vaultAddress)
{
    return RestApiMessageParser().parseSimpleResponse(
        impl_->signAndSendSync(RestEndpointType::UpdateIsolatedMargin,
                                   impl_->exchangeRequestBuilder.updateIsolatedMargin(request),
                                   vaultAddress));
}

SimpleResponse RestApi::approveAgent(const ApproveAgentRequest& request)
{
    return RestApiMessageParser().parseSimpleResponse(
        impl_->signAndSendSync(RestEndpointType::ApproveAgent,
                                   impl_->exchangeRequestBuilder.approveAgent(request)));
}


void RestApi::spotMetaAsync()
{
    impl_->signAndSend(RestEndpointType::SpotMeta, InfoRequestBuilder::spotMeta());
}

void RestApi::metaAsync(const std::optional<std::string>& dex)
{
    impl_->signAndSend(RestEndpointType::Meta, InfoRequestBuilder::meta(dex));
}

void RestApi::outcomeMetaAsync()
{
    impl_->signAndSend(RestEndpointType::OutcomeMeta, InfoRequestBuilder::outcomeMeta());
}

void RestApi::perpDexsAsync()
{
    impl_->signAndSend(RestEndpointType::PerpDexs, InfoRequestBuilder::perpDexs());
}

void RestApi::l2BookAsync(const std::string& coin,
                          const std::optional<int>& nSigFigs,
                          const std::optional<int>& mantissa)
{
    impl_->signAndSend(RestEndpointType::L2Book, InfoRequestBuilder::l2Book(coin, nSigFigs, mantissa));
}

void RestApi::candleSnapshotAsync(const std::string& coin,
                                  const std::string& interval,
                                  uint64_t startTime,
                                  uint64_t endTime)
{
    impl_->signAndSend(RestEndpointType::CandleSnapshot,
                       InfoRequestBuilder::candleSnapshot(coin, interval, startTime, endTime));
}

void RestApi::allMidsAsync(const std::optional<std::string>& dex)
{
    impl_->signAndSend(RestEndpointType::AllMids, InfoRequestBuilder::allMids(dex));
}

void RestApi::openOrdersAsync(const std::string& user, const std::optional<std::string>& dex)
{
    impl_->signAndSend(RestEndpointType::OpenOrders, InfoRequestBuilder::openOrders(user, dex));
}

void RestApi::orderStatusAsync(const std::string& user, const OrderId& oid)
{
    impl_->signAndSend(RestEndpointType::OrderStatus, InfoRequestBuilder::orderStatus(user, oid));
}

void RestApi::userFillsAsync(const std::string& user,
                             const std::optional<bool>& aggregateByTime,
                             const std::optional<std::string>& dex)
{
    impl_->signAndSend(RestEndpointType::UserFills, InfoRequestBuilder::userFills(user, aggregateByTime, dex));
}

void RestApi::userFillsByTimeAsync(const std::string& user,
                                   uint64_t startTime,
                                   const std::optional<uint64_t>& endTime,
                                   const std::optional<bool>& aggregateByTime,
                                   const std::optional<std::string>& dex)
{
    impl_->signAndSend(RestEndpointType::UserFillsByTime,
                       InfoRequestBuilder::userFillsByTime(user, startTime, endTime, aggregateByTime, dex));
}

void RestApi::clearinghouseStateAsync(const std::string& user, const std::optional<std::string>& dex)
{
    impl_->signAndSend(RestEndpointType::ClearinghouseState, InfoRequestBuilder::clearinghouseState(user, dex));
}

void RestApi::userRateLimitAsync(const std::string& user)
{
    impl_->signAndSend(RestEndpointType::UserRateLimit, InfoRequestBuilder::userRateLimit(user));
}

void RestApi::metaAndAssetCtxsAsync(const std::optional<std::string>& dex)
{
    impl_->signAndSend(RestEndpointType::MetaAndAssetCtxs, InfoRequestBuilder::metaAndAssetCtxs(dex));
}

void RestApi::spotMetaAndAssetCtxsAsync()
{
    impl_->signAndSend(RestEndpointType::SpotMetaAndAssetCtxs, InfoRequestBuilder::spotMetaAndAssetCtxs());
}

void RestApi::spotClearinghouseStateAsync(const std::string& user, const std::optional<std::string>& dex)
{
    impl_->signAndSend(RestEndpointType::SpotClearinghouseState, InfoRequestBuilder::spotClearinghouseState(user, dex));
}

void RestApi::frontendOpenOrdersAsync(const std::string& user, const std::optional<std::string>& dex)
{
    impl_->signAndSend(RestEndpointType::FrontendOpenOrders, InfoRequestBuilder::frontendOpenOrders(user, dex));
}

void RestApi::historicalOrdersAsync(const std::string& user)
{
    impl_->signAndSend(RestEndpointType::HistoricalOrders, InfoRequestBuilder::historicalOrders(user));
}

void RestApi::userTwapSliceFillsAsync(const std::string& user)
{
    impl_->signAndSend(RestEndpointType::UserTwapSliceFills, InfoRequestBuilder::userTwapSliceFills(user));
}

void RestApi::subAccountsAsync(const std::string& user)
{
    impl_->signAndSend(RestEndpointType::SubAccounts, InfoRequestBuilder::subAccounts(user));
}

void RestApi::userFeesAsync(const std::string& user)
{
    impl_->signAndSend(RestEndpointType::UserFees, InfoRequestBuilder::userFees(user));
}

void RestApi::maxBuilderFeeAsync(const std::string& user, const std::string& builder)
{
    impl_->signAndSend(RestEndpointType::MaxBuilderFee, InfoRequestBuilder::maxBuilderFee(user, builder));
}

void RestApi::approvedBuildersAsync(const std::string& user)
{
    impl_->signAndSend(RestEndpointType::ApprovedBuilders, InfoRequestBuilder::approvedBuilders(user));
}

void RestApi::placeOrderAsync(const std::vector<OrderRequest>& orders,
                               Grouping grouping,
                               const std::optional<Builder>& builder,
                               const std::optional<std::string>& vaultAddress)
{
    impl_->signAndSend(RestEndpointType::PlaceOrder,
                       impl_->exchangeRequestBuilder.placeOrder(orders, grouping, builder),
                       vaultAddress);
}

void RestApi::cancelOrderAsync(const std::vector<CancelRequest>& cancels,
                                const std::optional<std::string>& vaultAddress)
{
    impl_->signAndSend(RestEndpointType::CancelOrder,
                       impl_->exchangeRequestBuilder.cancelOrder(cancels),
                       vaultAddress);
}

void RestApi::cancelOrderByCloidAsync(const std::vector<CancelByCloidRequest>& cancels,
                                       const std::optional<std::string>& vaultAddress)
{
    impl_->signAndSend(RestEndpointType::CancelOrderByCloid,
                       impl_->exchangeRequestBuilder.cancelOrderByCloid(cancels),
                       vaultAddress);
}

void RestApi::scheduleCancelAsync(const std::optional<uint64_t>& time,
                                   const std::optional<std::string>& vaultAddress)
{
    impl_->signAndSend(RestEndpointType::ScheduleCancel,
                       impl_->exchangeRequestBuilder.scheduleCancel(time),
                       vaultAddress);
}

void RestApi::modifyOrderAsync(const ModifyRequest& modify,
                                const std::optional<std::string>& vaultAddress)
{
    impl_->signAndSend(RestEndpointType::ModifyOrder,
                       impl_->exchangeRequestBuilder.modifyOrder(modify),
                       vaultAddress);
}

void RestApi::batchModifyOrderAsync(const std::vector<ModifyRequest>& modifies,
                                     const std::optional<std::string>& vaultAddress)
{
    impl_->signAndSend(RestEndpointType::BatchModifyOrder,
                       impl_->exchangeRequestBuilder.batchModifyOrder(modifies),
                       vaultAddress);
}

void RestApi::updateLeverageAsync(const UpdateLeverageRequest& request,
                                   const std::optional<std::string>& vaultAddress)
{
    impl_->signAndSend(RestEndpointType::UpdateLeverage,
                       impl_->exchangeRequestBuilder.updateLeverage(request),
                       vaultAddress);
}

void RestApi::updateIsolatedMarginAsync(const UpdateIsolatedMarginRequest& request,
                                         const std::optional<std::string>& vaultAddress)
{
    impl_->signAndSend(RestEndpointType::UpdateIsolatedMargin,
                       impl_->exchangeRequestBuilder.updateIsolatedMargin(request),
                       vaultAddress);
}

void RestApi::approveAgentAsync(const ApproveAgentRequest& request)
{
    impl_->signAndSend(RestEndpointType::ApproveAgent,
                       impl_->exchangeRequestBuilder.approveAgent(request));
}

}
