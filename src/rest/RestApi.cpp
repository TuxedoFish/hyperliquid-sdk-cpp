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

    void signAndSend(RestEndpointType type, nlohmann::ordered_json body,
                     const std::optional<std::string>& vaultAddress = std::nullopt,
                     const std::optional<uint64_t>& expiresAfter = std::nullopt)
    {
        auto effectiveVault = vaultAddress ? vaultAddress : config.vaultAddress;
        auto prepared = Signing::prepareBody(config, type, std::move(body), effectiveVault, expiresAfter);
        std::string serialized = prepared.dump();

        auto session = std::make_shared<HttpSession>(
            ioc, sslCtx, host, port, toPath(type),
            [this, type](const std::string& responseBody, beast::error_code ec) {
                if (ec) {
                    getLogger()->error("RestApi: error for {}: {}", toString(type), ec.message());
                    listener.onError(type, ec.message());
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
        auto effectiveVault = vaultAddress ? vaultAddress : config.vaultAddress;
        auto prepared = Signing::prepareBody(config, type, std::move(body), effectiveVault, expiresAfter);
        std::string serialized = prepared.dump();
        getLogger()->debug("{}", serialized);

        auto promise = std::make_shared<std::promise<std::string>>();
        auto future = promise->get_future();

        auto session = std::make_shared<HttpSession>(
            ioc, sslCtx, host, port, toPath(type),
            [promise](const std::string& responseBody, beast::error_code ec) {
                if (ec) {
                    promise->set_exception(std::make_exception_ptr(
                        RestApiTransportError("RestApi: " + ec.message())));
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

}
