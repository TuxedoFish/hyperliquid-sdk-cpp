#include "hyperliquid/rest/RestApi.h"
#include "hyperliquid/rest/RestApiListener.h"
#include "hyperliquid/rest/RestApiMessageParser.h"
#include "HttpSession.h"
#include "InfoRequestBuilder.h"
#include "ExchangeRequestBuilder.h"
#include "SymbolMap.h"
#include "signing/Signing.h"

#include <chrono>
#include <future>
#include <memory>
#include <set>
#include <thread>

#include <boost/asio/executor_work_guard.hpp>

#include <nlohmann/json.hpp>

#include "Logger.h"

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
    Wallet wallet;
    bool authenticated = false;
    Environment env;
    std::set<std::string> enabledDexes;
    SymbolMap symbolMap;
    ExchangeRequestBuilder exchangeRequestBuilder;

    Impl(const RestApiConfig& config, RestApiListener& listener)
        : work(net::make_work_guard(ioc))
        , sslCtx(ssl::context::tlsv12_client)
        , host(toInfoEndpoint(config.env).host)
        , port(toInfoEndpoint(config.env).port)
        , listener(listener)
        , env(config.env)
        , enabledDexes(config.dexes)
        , exchangeRequestBuilder(symbolMap)
    {
        sslCtx.set_default_verify_paths();
        sslCtx.set_verify_mode(ssl::verify_peer);
        thread = std::thread([this]() { ioc.run(); });

        if (config.wallet)
        {
            wallet = *config.wallet;
            authenticated = true;
        }

        buildSymbolMap();
    }

    ~Impl()
    {
        work.reset();
        ioc.stop();
        if (thread.joinable()) thread.join();
    }

    void buildSymbolMap()
    {
        RestApiMessageParser parser;

        auto defaultMetaResponse = parser.parseMeta(
            signAndSendSync(RestEndpointType::Meta, InfoRequestBuilder::meta()));
        int index = 0;
        for (const auto& asset : defaultMetaResponse.universe)
        {
            symbolMap.add(asset.name, index);
            index++;
        }

        auto spotMetaResponse = parser.parseSpotMeta(
            signAndSendSync(RestEndpointType::SpotMeta, InfoRequestBuilder::spotMeta()));
        for (const auto& token : spotMetaResponse.tokens)
        {
            symbolMap.add(token.name, token.index + 10000);
        }

        if (enabledDexes.empty()) return;

        auto dexesResponse = parser.parsePerpDexs(
            signAndSendSync(RestEndpointType::PerpDexs, InfoRequestBuilder::perpDexs()));

        int perpIdx = 0;
        for (const auto& dex : dexesResponse.dexes)
        {
            if (enabledDexes.count(dex.name) == 0)
            {
                perpIdx++;
                continue;
            }

            auto dexMetaResponse = parser.parseMeta(
                signAndSendSync(RestEndpointType::Meta, InfoRequestBuilder::meta(dex.name)));
            index = 0;
            for (const auto& asset : dexMetaResponse.universe)
            {
                symbolMap.add(asset.name, 100000 + (perpIdx * 10000) + index);
                index++;
            }
            perpIdx++;
        }
    }

    nlohmann::ordered_json prepareBody(RestEndpointType type, nlohmann::ordered_json body,
                               const std::optional<std::string>& vaultAddress = std::nullopt,
                               const std::optional<uint64_t>& expiresAfter = std::nullopt)
    {
        if (vaultAddress) body["vaultAddress"] = *vaultAddress;
        if (expiresAfter) body["expiresAfter"] = *expiresAfter;

        if (isAuthenticated(type))
        {
            uint64_t nonce = static_cast<uint64_t>(
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now().time_since_epoch()).count());
            body["nonce"] = nonce;

            bool isMainnet = (env == Environment::Mainnet);
            auto action = body["action"];
            auto signature = Signing::signL1Action(
                wallet, action, vaultAddress, nonce, expiresAfter, isMainnet);

            nlohmann::ordered_json signatureJson;
            signatureJson["r"] = signature.r;
            signatureJson["s"] = signature.s;
            signatureJson["v"] = signature.v;
            body["signature"] = signatureJson;
        }

        return body;
    }

    void signAndSend(RestEndpointType type, nlohmann::ordered_json body,
                     const std::optional<std::string>& vaultAddress = std::nullopt,
                     const std::optional<uint64_t>& expiresAfter = std::nullopt)
    {
        auto prepared = prepareBody(type, body, vaultAddress, expiresAfter);
        std::string serialized = prepared.dump();

        auto session = std::make_shared<HttpSession>(
            ioc, sslCtx, host, port, toPath(type),
            [this, type](const std::string& responseBody, beast::error_code ec) {
                if (ec) {
                    getLogger()->error("RestApi: error for {}: {}", toString(type), ec.message());
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
        auto prepared = prepareBody(type, body, vaultAddress, expiresAfter);
        std::string serialized = prepared.dump();
        getLogger()->debug("{}", serialized);

        auto promise = std::make_shared<std::promise<std::string>>();
        auto future = promise->get_future();

        auto session = std::make_shared<HttpSession>(
            ioc, sslCtx, host, port, toPath(type),
            [promise](const std::string& responseBody, beast::error_code ec) {
                if (ec) {
                    promise->set_exception(std::make_exception_ptr(
                        std::runtime_error("RestApi: " + ec.message())));
                    return;
                }
                promise->set_value(responseBody);
            });

        session->run(serialized);
        return future.get();
    }
};

RestApi::RestApi(const RestApiConfig& config)
    : impl_(std::make_unique<Impl>(config, defaultListener))
{
}

RestApi::RestApi(const RestApiConfig& config, RestApiListener& listener)
    : impl_(std::make_unique<Impl>(config, listener))
{
}

RestApi::~RestApi() = default;

std::string RestApi::spotMeta()
{
    return impl_->signAndSendSync(RestEndpointType::SpotMeta, InfoRequestBuilder::spotMeta());
}

std::string RestApi::meta(const std::optional<std::string>& dex)
{
    return impl_->signAndSendSync(RestEndpointType::Meta, InfoRequestBuilder::meta(dex));
}

std::string RestApi::perpDexs()
{
    return impl_->signAndSendSync(RestEndpointType::PerpDexs, InfoRequestBuilder::perpDexs());
}

std::string RestApi::placeOrder(const std::vector<OrderRequest>& orders,
                                Grouping grouping,
                                const std::optional<Builder>& builder)
{
    return impl_->signAndSendSync(RestEndpointType::PlaceOrder,
                                  impl_->exchangeRequestBuilder.placeOrder(orders, grouping, builder));
}

void RestApi::spotMetaAsync()
{
    impl_->signAndSend(RestEndpointType::SpotMeta, InfoRequestBuilder::spotMeta());
}

void RestApi::metaAsync(const std::optional<std::string>& dex)
{
    impl_->signAndSend(RestEndpointType::Meta, InfoRequestBuilder::meta(dex));
}

void RestApi::perpDexsAsync()
{
    impl_->signAndSend(RestEndpointType::PerpDexs, InfoRequestBuilder::perpDexs());
}

void RestApi::placeOrderAsync(const std::vector<OrderRequest>& orders,
                               Grouping grouping,
                               const std::optional<Builder>& builder)
{
    impl_->signAndSend(RestEndpointType::PlaceOrder, impl_->exchangeRequestBuilder.placeOrder(orders, grouping, builder));
}

std::string RestApi::cancelOrder(const std::vector<CancelRequest>& cancels)
{
    return impl_->signAndSendSync(RestEndpointType::CancelOrder,
                                   impl_->exchangeRequestBuilder.cancelOrder(cancels));
}

std::string RestApi::cancelOrderByCloid(const std::vector<CancelByCloidRequest>& cancels)
{
    return impl_->signAndSendSync(RestEndpointType::CancelOrderByCloid,
                                   impl_->exchangeRequestBuilder.cancelOrderByCloid(cancels));
}

std::string RestApi::modifyOrder(const ModifyRequest& modify)
{
    return impl_->signAndSendSync(RestEndpointType::ModifyOrder,
                                   impl_->exchangeRequestBuilder.modifyOrder(modify));
}

std::string RestApi::batchModifyOrder(const std::vector<ModifyRequest>& modifies)
{
    return impl_->signAndSendSync(RestEndpointType::BatchModifyOrder,
                                   impl_->exchangeRequestBuilder.batchModifyOrder(modifies));
}

void RestApi::cancelOrderAsync(const std::vector<CancelRequest>& cancels)
{
    impl_->signAndSend(RestEndpointType::CancelOrder, impl_->exchangeRequestBuilder.cancelOrder(cancels));
}

void RestApi::cancelOrderByCloidAsync(const std::vector<CancelByCloidRequest>& cancels)
{
    impl_->signAndSend(RestEndpointType::CancelOrderByCloid, impl_->exchangeRequestBuilder.cancelOrderByCloid(cancels));
}

void RestApi::modifyOrderAsync(const ModifyRequest& modify)
{
    impl_->signAndSend(RestEndpointType::ModifyOrder, impl_->exchangeRequestBuilder.modifyOrder(modify));
}

void RestApi::batchModifyOrderAsync(const std::vector<ModifyRequest>& modifies)
{
    impl_->signAndSend(RestEndpointType::BatchModifyOrder, impl_->exchangeRequestBuilder.batchModifyOrder(modifies));
}

}
