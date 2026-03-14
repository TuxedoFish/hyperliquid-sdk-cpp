#pragma once

#include <atomic>
#include <memory>
#include <string>
#include <queue>
#include <thread>

#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/steady_timer.hpp>

#include "hyperliquid/config/Config.h"

namespace hyperliquid {
namespace internal {

namespace beast = boost::beast;
namespace websocket = beast::websocket;
namespace net = boost::asio;
namespace ssl = boost::asio::ssl;
using tcp = boost::asio::ip::tcp;

class WSListener {
public:
    virtual ~WSListener() = default;
    virtual void onWsMessage(std::string& message) = 0;
    virtual void onWsConnected() = 0;
    virtual void onWsDisconnected(bool hasError, const std::string& errMsg) = 0;
};

class WebsocketRunner {
public:
    WebsocketRunner(ApiConfig& config, WSListener& listener);
    ~WebsocketRunner();

    void send(const std::string& message);

    net::io_context& getIoContext();

    void start();
    void stop();

private:
    void doResolve();
    void onResolve(beast::error_code ec, tcp::resolver::results_type results);
    void onConnect(beast::error_code ec, tcp::resolver::results_type::endpoint_type ep);
    void onSslHandshake(beast::error_code ec);
    void onWsHandshake(beast::error_code ec);
    void doRead();
    void onRead(beast::error_code errorCode, std::size_t bytesTransferred);
    void doWrite();
    void onWrite(beast::error_code ec, std::size_t bytesTransferred);
    void doClose();
    void scheduleReconnect();
    void doReconnect();

    void setupControlCallback();
    void startPingTimer();
    void doPing();

    WSListener& listener_;

    net::io_context ioc_;
    ssl::context sslCtx_;
    tcp::resolver resolver_;
    std::unique_ptr<websocket::stream<beast::ssl_stream<beast::tcp_stream>>> ws_;
    beast::flat_buffer readBuf_;

    std::string host_;
    std::string port_;
    std::string path_;

    std::queue<std::string> writeQueue_;
    bool writing_ = false;
    std::atomic<bool> connected_{false};
    std::atomic<bool> stopping_{false};

    std::unique_ptr<net::steady_timer> reconnectTimer_;
    int reconnectAttempts_{0};
    static constexpr int MAX_BACKOFF_SECS = 60;

    std::unique_ptr<net::steady_timer> pingTimer_;
    std::chrono::steady_clock::time_point lastPongTime_;
    bool pendingPong_{false};
    static constexpr int PING_INTERVAL_SECS = 30;
    static constexpr int PONG_TIMEOUT_SECS = 10;
};

}  // namespace internal
}  // namespace hyperliquid
