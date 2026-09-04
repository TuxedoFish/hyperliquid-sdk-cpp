#include "HttpSession.h"

#include <chrono>
#include "../config/Logger.h"

namespace hyperliquid {

HttpSession::HttpSession(net::io_context& ioc, ssl::context& sslCtx,
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

void HttpSession::run(const std::string& body)
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

void HttpSession::onResolve(beast::error_code ec, tcp::resolver::results_type results)
{
    if (ec) {
        onComplete_("", 0, ec);
        return;
    }

    if (!SSL_set_tlsext_host_name(stream_.native_handle(), host_.c_str())) {
        ec = beast::error_code(static_cast<int>(::ERR_get_error()), net::error::get_ssl_category());
        onComplete_("", 0, ec);
        return;
    }

    beast::get_lowest_layer(stream_).expires_after(std::chrono::seconds(10));
    beast::get_lowest_layer(stream_).async_connect(results,
        [self = shared_from_this()](beast::error_code ec, tcp::resolver::results_type::endpoint_type) {
            self->onConnect(ec);
        });
}

void HttpSession::onConnect(beast::error_code ec)
{
    if (ec) {
        onComplete_("", 0, ec);
        return;
    }

    beast::get_lowest_layer(stream_).expires_after(std::chrono::seconds(10));
    stream_.async_handshake(ssl::stream_base::client,
        [self = shared_from_this()](beast::error_code ec) {
            self->onSslHandshake(ec);
        });
}

void HttpSession::onSslHandshake(beast::error_code ec)
{
    if (ec) {
        onComplete_("", 0, ec);
        return;
    }

    beast::get_lowest_layer(stream_).expires_after(std::chrono::seconds(10));
    http::async_write(stream_, req_,
        [self = shared_from_this()](beast::error_code ec, std::size_t) {
            self->onWrite(ec);
        });
}

void HttpSession::onWrite(beast::error_code ec)
{
    if (ec) {
        onComplete_("", 0, ec);
        return;
    }

    http::async_read(stream_, buffer_, res_,
        [self = shared_from_this()](beast::error_code ec, std::size_t) {
            self->onRead(ec);
        });
}

void HttpSession::onRead(beast::error_code ec)
{
    if (ec) {
        onComplete_("", 0, ec);
        return;
    }

    onComplete_(res_.body(), res_.result_int(), {});

    beast::get_lowest_layer(stream_).expires_after(std::chrono::seconds(5));
    stream_.async_shutdown(
        [self = shared_from_this()](beast::error_code ec) {
            if (ec && ec != net::error::eof && ec != beast::errc::not_connected
                && ec != net::ssl::error::stream_truncated) {
                getLogger()->error("HttpSession shutdown error: {}", ec.message());
            }
        });
}

}
