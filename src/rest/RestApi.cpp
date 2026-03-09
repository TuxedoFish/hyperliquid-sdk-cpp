#include "hyperliquid/rest/RestApi.h"
#include "hyperliquid/rest/RestApiListener.h"

#include <chrono>
#include <functional>
#include <future>
#include <iostream>
#include <memory>
#include <thread>

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/executor_work_guard.hpp>

#include <nlohmann/json.hpp>

namespace hyperliquid {

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
namespace ssl = boost::asio::ssl;
using tcp = boost::asio::ip::tcp;

using OnComplete = std::function<void(const std::string&, beast::error_code)>;

class HttpSession : public std::enable_shared_from_this<HttpSession> {
public:
    HttpSession(net::io_context& ioc, ssl::context& sslCtx,
                const std::string& host, const std::string& port,
                const std::string& path,
                OnComplete onComplete)
        : resolver_(net::make_strand(ioc))
        , stream_(net::make_strand(ioc), sslCtx)
        , host_(host)
        , port_(port)
        , path_(path)
        , onComplete_(onComplete)
    {
    }

    void run(const std::string& body)
    {
        req_.method(http::verb::post);
        req_.target(path_);
        req_.version(11);
        req_.set(http::field::host, host_);
        req_.set(http::field::content_type, "application/json");
        req_.set(http::field::user_agent, "hyperliquid-sdk-cpp/0.1");
        req_.body() = body;
        req_.prepare_payload();

        resolver_.async_resolve(host_, port_,
            [self = shared_from_this()](beast::error_code ec, tcp::resolver::results_type results) {
                self->onResolve(ec, results);
            });
    }

private:
    void onResolve(beast::error_code ec, tcp::resolver::results_type results)
    {
        if (ec) {
            onComplete_("", ec);
            return;
        }

        if (!SSL_set_tlsext_host_name(stream_.native_handle(), host_.c_str())) {
            ec = beast::error_code(static_cast<int>(::ERR_get_error()), net::error::get_ssl_category());
            onComplete_("", ec);
            return;
        }

        beast::get_lowest_layer(stream_).expires_after(std::chrono::seconds(10));
        beast::get_lowest_layer(stream_).async_connect(results,
            [self = shared_from_this()](beast::error_code ec, tcp::resolver::results_type::endpoint_type) {
                self->onConnect(ec);
            });
    }

    void onConnect(beast::error_code ec)
    {
        if (ec) {
            onComplete_("", ec);
            return;
        }

        beast::get_lowest_layer(stream_).expires_after(std::chrono::seconds(10));
        stream_.async_handshake(ssl::stream_base::client,
            [self = shared_from_this()](beast::error_code ec) {
                self->onSslHandshake(ec);
            });
    }

    void onSslHandshake(beast::error_code ec)
    {
        if (ec) {
            onComplete_("", ec);
            return;
        }

        beast::get_lowest_layer(stream_).expires_after(std::chrono::seconds(10));
        http::async_write(stream_, req_,
            [self = shared_from_this()](beast::error_code ec, std::size_t) {
                self->onWrite(ec);
            });
    }

    void onWrite(beast::error_code ec)
    {
        if (ec) {
            onComplete_("", ec);
            return;
        }

        http::async_read(stream_, buffer_, res_,
            [self = shared_from_this()](beast::error_code ec, std::size_t) {
                self->onRead(ec);
            });
    }

    void onRead(beast::error_code ec)
    {
        if (ec) {
            onComplete_("", ec);
            return;
        }

        onComplete_(res_.body(), {});

        beast::get_lowest_layer(stream_).expires_after(std::chrono::seconds(5));
        stream_.async_shutdown(
            [self = shared_from_this()](beast::error_code ec) {
                if (ec && ec != net::error::eof && ec != beast::errc::not_connected
                    && ec != net::ssl::error::stream_truncated) {
                    std::cerr << "InfoApi shutdown error: " << ec.message() << std::endl;
                }
            });
    }

    tcp::resolver resolver_;
    beast::ssl_stream<beast::tcp_stream> stream_;
    beast::flat_buffer buffer_;
    http::request<http::string_body> req_;
    http::response<http::string_body> res_;
    std::string host_;
    std::string port_;
    std::string path_;
    OnComplete onComplete_;
};

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

    Impl(Environment env, RestApiListener& listener, Wallet wallet)
        : work(net::make_work_guard(ioc))
        , sslCtx(ssl::context::tlsv12_client)
        , host(toInfoEndpoint(env).host)
        , port(toInfoEndpoint(env).port)
        , listener(listener)
        , wallet(wallet)
    {
        sslCtx.set_default_verify_paths();
        sslCtx.set_verify_mode(ssl::verify_peer);
        thread = std::thread([this]() { ioc.run(); });
        authenticated = true;
    }

    Impl(Environment env, RestApiListener& listener)
        : work(net::make_work_guard(ioc))
        , sslCtx(ssl::context::tlsv12_client)
        , host(toInfoEndpoint(env).host)
        , port(toInfoEndpoint(env).port)
        , listener(listener)
    {
        sslCtx.set_default_verify_paths();
        sslCtx.set_verify_mode(ssl::verify_peer);
        thread = std::thread([this]() { ioc.run(); });
    }

    Impl(Environment env)
        : work(net::make_work_guard(ioc))
        , sslCtx(ssl::context::tlsv12_client)
        , host(toInfoEndpoint(env).host)
        , port(toInfoEndpoint(env).port)
        , listener(defaultListener)
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

    nlohmann::json prepareBody(RestEndpointType type, nlohmann::json body,
                               const std::optional<std::string>& vaultAddress = std::nullopt,
                               const std::optional<uint64_t>& expiresAfter = std::nullopt)
    {
        if (vaultAddress) body["vaultAddress"] = *vaultAddress;
        if (expiresAfter) body["expiresAfter"] = *expiresAfter;

        if (isAuthenticated(type))
        {
            body["nonce"] = static_cast<uint64_t>(
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now().time_since_epoch()).count());
            body["signature"] = "xyz";
        }

        return body;
    }

    void signAndSend(RestEndpointType type, nlohmann::json body,
                     const std::optional<std::string>& vaultAddress = std::nullopt,
                     const std::optional<uint64_t>& expiresAfter = std::nullopt)
    {
        auto prepared = prepareBody(type, body, vaultAddress, expiresAfter);
        std::string serialized = prepared.dump();

        auto session = std::make_shared<HttpSession>(
            ioc, sslCtx, host, port, toPath(type),
            [this, type](const std::string& responseBody, beast::error_code ec) {
                if (ec) {
                    std::cerr << "RestApi: error for " << toString(type) << ": " << ec.message() << std::endl;
                    return;
                }
                listener.onMessage(responseBody, type);
            });

        session->run(serialized);
    }

    std::string signAndSendSync(RestEndpointType type, nlohmann::json body,
                                const std::optional<std::string>& vaultAddress = std::nullopt,
                                const std::optional<uint64_t>& expiresAfter = std::nullopt)
    {
        auto prepared = prepareBody(type, body, vaultAddress, expiresAfter);
        std::string serialized = prepared.dump();

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

RestApi::RestApi(Environment env, RestApiListener& listener, Wallet wallet)
    : impl_(std::make_unique<Impl>(env, listener))
{
}

RestApi::RestApi(Environment env, RestApiListener& listener)
    : impl_(std::make_unique<Impl>(env, listener))
{
}

RestApi::RestApi(Environment env)
    : impl_(std::make_unique<Impl>(env))
{
}

RestApi::~RestApi() = default;

static nlohmann::json buildSpotMetaBody()
{
    nlohmann::json body;
    body["type"] = toString(RestEndpointType::SpotMeta);
    return body;
}

static nlohmann::json buildMetaBody(const std::optional<std::string>& dex)
{
    nlohmann::json body;
    body["type"] = toString(RestEndpointType::Meta);
    if (dex) body["dex"] = *dex;
    return body;
}

static nlohmann::json buildPerpDexsBody()
{
    nlohmann::json body;
    body["type"] = toString(RestEndpointType::PerpDexs);
    return body;
}

static nlohmann::json buildPlaceOrderBody(const std::vector<OrderRequest>& orders,
                                           Grouping grouping,
                                           const std::optional<Builder>& builder)
{
    nlohmann::json ordersJson = nlohmann::json::array();
    for (const auto& order : orders)
    {
        nlohmann::json orderJson;
        orderJson["a"] = order.asset;
        orderJson["b"] = order.isBuy;
        orderJson["p"] = order.price;
        orderJson["s"] = order.size;
        orderJson["r"] = order.reduceOnly;

        if (order.limit)
        {
            orderJson["t"] = {{"limit", {{"tif", toString(order.limit->tif)}}}};
        }
        else if (order.trigger)
        {
            orderJson["t"] = {{"trigger", {
                {"isMarket", order.trigger->isMarket},
                {"triggerPx", order.trigger->triggerPx},
                {"tpsl", toString(order.trigger->tpsl)}
            }}};
        }

        if (order.cloid) orderJson["c"] = *order.cloid;

        ordersJson.push_back(orderJson);
    }

    nlohmann::json action;
    action["type"] = "order";
    action["orders"] = ordersJson;
    action["grouping"] = toString(grouping);

    if (builder)
    {
        action["builder"] = {{"b", builder->address}, {"f", builder->fee}};
    }

    nlohmann::json body;
    body["action"] = action;
    return body;
}

// Sync methods

std::string RestApi::spotMeta()
{
    return impl_->signAndSendSync(RestEndpointType::SpotMeta, buildSpotMetaBody());
}

std::string RestApi::meta(const std::optional<std::string>& dex)
{
    return impl_->signAndSendSync(RestEndpointType::Meta, buildMetaBody(dex));
}

std::string RestApi::perpDexs()
{
    return impl_->signAndSendSync(RestEndpointType::PerpDexs, buildPerpDexsBody());
}

std::string RestApi::placeOrder(const std::vector<OrderRequest>& orders,
                                Grouping grouping,
                                const std::optional<Builder>& builder)
{
    return impl_->signAndSendSync(RestEndpointType::PlaceOrder,
                                  buildPlaceOrderBody(orders, grouping, builder));
}

// Async methods

void RestApi::spotMetaAsync()
{
    return impl_->signAndSend(RestEndpointType::SpotMeta, buildSpotMetaBody());
}

void RestApi::metaAsync(const std::optional<std::string>& dex)
{
    impl_->signAndSend(RestEndpointType::Meta, buildMetaBody(dex));
}

void RestApi::perpDexsAsync()
{
    impl_->signAndSend(RestEndpointType::PerpDexs, buildPerpDexsBody());
}

void RestApi::placeOrderAsync(const std::vector<OrderRequest>& orders,
                               Grouping grouping,
                               const std::optional<Builder>& builder)
{
    impl_->signAndSend(RestEndpointType::PlaceOrder,
                       buildPlaceOrderBody(orders, grouping, builder));
}

}
