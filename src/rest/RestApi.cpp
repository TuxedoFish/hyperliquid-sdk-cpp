#include "hyperliquid/rest/RestApi.h"
#include "hyperliquid/rest/RestApiListener.h"

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

// Per-request session that lives through the async chain via shared_ptr
class HttpSession : public std::enable_shared_from_this<HttpSession> {
public:
    HttpSession(net::io_context& ioc, ssl::context& sslCtx,
                const std::string& host, const std::string& port,
                RestApiListener& listener, RestEndpointType type)
        : resolver_(net::make_strand(ioc))
        , stream_(net::make_strand(ioc), sslCtx)
        , host_(host)
        , port_(port)
        , listener_(listener)
        , type_(type)
    {
    }

    void run(const std::string& body)
    {
        req_.method(http::verb::post);
        req_.target("/info");
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
            std::cerr << "InfoApi resolve error: " << ec.message() << std::endl;
            return;
        }

        if (!SSL_set_tlsext_host_name(stream_.native_handle(), host_.c_str())) {
            ec = beast::error_code(static_cast<int>(::ERR_get_error()), net::error::get_ssl_category());
            std::cerr << "InfoApi SNI error: " << ec.message() << std::endl;
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
            std::cerr << "InfoApi connect error: " << ec.message() << std::endl;
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
            std::cerr << "InfoApi SSL handshake error: " << ec.message() << std::endl;
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
            std::cerr << "InfoApi write error: " << ec.message() << std::endl;
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
            std::cerr << "InfoApi read error: " << ec.message() << std::endl;
            return;
        }

        listener_.onMessage(res_.body(), type_);

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
    RestApiListener& listener_;
    RestEndpointType type_;
};

// InfoApi pimpl
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

    ~Impl()
    {
        work.reset();
        ioc.stop();
        if (thread.joinable()) thread.join();
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

RestApi::~RestApi() = default;

void RestApi::sendRequest(RestEndpointType type, const std::map<std::string, std::string>& params)
{
    nlohmann::json body;
    body["type"] = toString(type);
    for (const auto& [key, value] : params) {
        body[key] = value;
    }

    if (isAuthenticated(type))
    {
       if (!impl_->authenticated)
       {
           std::cerr << "RestApi: Not authenticated - rejecting " << toString(type) << std::endl;
           return;
       }

        // TODO: Add the signature in order to send the request
    }

    auto session = std::make_shared<HttpSession>(
        impl_->ioc, impl_->sslCtx,
        impl_->host, impl_->port,
        impl_->listener, type);

    session->run(body.dump());
}

}
