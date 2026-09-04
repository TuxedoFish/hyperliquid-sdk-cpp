#pragma once

#include <functional>
#include <memory>
#include <string>

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/asio/strand.hpp>

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
                OnComplete onComplete);

    void run(const std::string& body);

private:
    void onResolve(beast::error_code ec, tcp::resolver::results_type results);
    void onConnect(beast::error_code ec);
    void onSslHandshake(beast::error_code ec);
    void onWrite(beast::error_code ec);
    void onRead(beast::error_code ec);

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

} // namespace hyperliquid
