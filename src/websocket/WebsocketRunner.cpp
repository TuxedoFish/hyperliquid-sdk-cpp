#include "WebsocketRunner.h"
#include <algorithm>
#include "../config/Logger.h"

namespace hyperliquid {
namespace internal {

WebsocketRunner::WebsocketRunner(ApiConfig& config, WSListener& listener)
    : listener_(listener)
    , sslCtx_(ssl::context::tlsv12_client)
    , resolver_(net::make_strand(ioc_))
    , ws_(std::make_unique<websocket::stream<beast::ssl_stream<beast::tcp_stream>>>(net::make_strand(ioc_), sslCtx_)) {
    auto endpoint = toWsEndpoint(config.env);
    host_ = endpoint.host;
    port_ = endpoint.port;
    path_ = endpoint.path;

    sslCtx_.set_default_verify_paths();
    sslCtx_.set_verify_mode(ssl::verify_peer);
}

WebsocketRunner::~WebsocketRunner() {
    stop();
}

net::io_context& WebsocketRunner::getIoContext() { return ioc_; }

void WebsocketRunner::onPongReceived() {
    lastPongTime_ = std::chrono::steady_clock::now();
    pendingPong_ = false;
}

void WebsocketRunner::send(const std::string& message) {
    getLogger()->debug("ws send: {}", message);
    net::post(ws_->get_executor(), [this, msg = message]() {
        writeQueue_.push(msg);
        if (connected_ && !writing_) {
            doWrite();
        }
    });
}

void WebsocketRunner::start() {
    doResolve();
    ioc_.run();
}

void WebsocketRunner::stop() {
    if (reconnectTimer_) reconnectTimer_->cancel();
    if (pingTimer_) pingTimer_->cancel();
    if (!ioc_.stopped() && !stopping_) {
        net::post(ws_->get_executor(), [this]() { doClose(); });
    }
}

void WebsocketRunner::doResolve() {
    resolver_.async_resolve(host_, port_,
        [this](beast::error_code ec, tcp::resolver::results_type results) {
            onResolve(ec, results);
        });
}

void WebsocketRunner::onResolve(beast::error_code ec, tcp::resolver::results_type results) {
    if (ec) {
        getLogger()->error("resolve error: {}", ec.message());
        scheduleReconnect();
        return;
    }

    beast::get_lowest_layer(*ws_).expires_after(std::chrono::seconds(10));

    beast::get_lowest_layer(*ws_).async_connect(results,
        [this](beast::error_code ec, tcp::resolver::results_type::endpoint_type ep) {
            onConnect(ec, ep);
        });
}

void WebsocketRunner::onConnect(beast::error_code ec, tcp::resolver::results_type::endpoint_type) {
    if (ec) {
        getLogger()->error("connect error: {}", ec.message());
        scheduleReconnect();
        return;
    }

    if (!SSL_set_tlsext_host_name(ws_->next_layer().native_handle(), host_.c_str())) {
        ec = beast::error_code(static_cast<int>(::ERR_get_error()), net::error::get_ssl_category());
        getLogger()->error("SNI error: {}", ec.message());
        scheduleReconnect();
        return;
    }

    beast::get_lowest_layer(*ws_).expires_after(std::chrono::seconds(10));

    ws_->next_layer().async_handshake(ssl::stream_base::client,
        [this](beast::error_code ec) { onSslHandshake(ec); });
}

void WebsocketRunner::onSslHandshake(beast::error_code ec) {
    if (ec) {
        getLogger()->error("SSL handshake error: {}", ec.message());
        scheduleReconnect();
        return;
    }

    beast::get_lowest_layer(*ws_).expires_never();

    ws_->set_option(websocket::stream_base::decorator(
        [](websocket::request_type& req) {
            req.set(boost::beast::http::field::user_agent, "hyperliquid-sdk-cpp/0.1");
        }));

    ws_->async_handshake(host_, path_,
        [this](beast::error_code ec) { onWsHandshake(ec); });
}

void WebsocketRunner::onWsHandshake(beast::error_code ec) {
    if (ec) {
        getLogger()->error("WebSocket handshake error: {}", ec.message());
        scheduleReconnect();
        return;
    }

    getLogger()->info("WebSocket connected successfully");

    if (reconnectTimer_) {
        reconnectTimer_->cancel();
        reconnectTimer_.reset();
    }

    setupControlCallback();
    lastPongTime_ = std::chrono::steady_clock::now();
    pendingPong_ = false;

    connected_ = true;
    reconnectAttempts_ = 0;
    listener_.onWsConnected();

    if (!writeQueue_.empty()) {
        doWrite();
    }

    startPingTimer();
    doRead();
}

void WebsocketRunner::doRead() {
    ws_->async_read(readBuf_,
        [this](beast::error_code ec, std::size_t bytesTransferred) {
            onRead(ec, bytesTransferred);
        });
}

void WebsocketRunner::onRead(beast::error_code errorCode, std::size_t) {
    // Stale callback from a cancelled/replaced stream — ignore
    if (!connected_ && !stopping_) return;

    if (errorCode) {
        if (stopping_) {
            // read cancelled, now safe to close
            getLogger()->info("Closing websocket...");
            ws_->async_close(websocket::close_code::normal,
                [this](beast::error_code ec) {
                    if (ec) {
                        beast::error_code tec;
                        beast::get_lowest_layer(*ws_).socket().close(tec);
                    }
                    listener_.onWsDisconnected(false, "");
                    connected_ = false;
                    stopping_ = false;
                });
        } else if (errorCode == websocket::error::closed) {
            getLogger()->error("Unexpected websocket close (connected={} writing={} queueSize={})",
                               connected_.load(), writing_, writeQueue_.size());
            listener_.onWsDisconnected(true, errorCode.message());
            connected_ = false;
            scheduleReconnect();
        } else {
            getLogger()->error("Unexpected read error: {} (connected={} writing={} queueSize={})",
                               errorCode.message(), connected_.load(), writing_, writeQueue_.size());
            listener_.onWsDisconnected(true, errorCode.message());
            connected_ = false;
            scheduleReconnect();
        }
        return;
    }

    auto data = readBuf_.data();
    std::string sv(static_cast<const char*>(data.data()), data.size());
    listener_.onWsMessage(sv);

    readBuf_.consume(readBuf_.size());
    doRead();
}

void WebsocketRunner::doWrite() {
    if (writeQueue_.empty()) {
        writing_ = false;
        return;
    }

    writing_ = true;
    ws_->async_write(net::buffer(writeQueue_.front()),
        [this](beast::error_code ec, std::size_t bytesTransferred) {
            onWrite(ec, bytesTransferred);
        });
}

void WebsocketRunner::onWrite(beast::error_code ec, std::size_t) {
    if (!connected_) return;

    if (ec) {
        getLogger()->error("write error: {} (connected={} queueSize={})", ec.message(), connected_.load(), writeQueue_.size());
        writing_ = false;
        return;
    }

    writeQueue_.pop();
    doWrite();
}

void WebsocketRunner::doClose() {
    if (!connected_ || stopping_) return;
    if (pingTimer_) pingTimer_->cancel();
    stopping_ = true;
    getLogger()->info("Waiting to close websocket...");
    beast::get_lowest_layer(*ws_).cancel();
}

void WebsocketRunner::scheduleReconnect() {
    if (stopping_) return;
    if (pingTimer_) pingTimer_->cancel();
    if (reconnectTimer_) reconnectTimer_->cancel();

    reconnectAttempts_++;
    int delaySecs = std::min(1 << reconnectAttempts_, MAX_BACKOFF_SECS);
    getLogger()->info("Reconnecting in {}s (attempt {})", delaySecs, reconnectAttempts_);

    reconnectTimer_ = std::make_unique<net::steady_timer>(ioc_, std::chrono::seconds(delaySecs));
    reconnectTimer_->async_wait([this](const boost::system::error_code& ec) {
        if (!ec && !stopping_) {
            doReconnect();
        }
    });
}

void WebsocketRunner::doReconnect() {
    if (connected_) return;

    if (reconnectTimer_) {
        reconnectTimer_->cancel();
        reconnectTimer_.reset();
    }

    getLogger()->info("doReconnect: clearing state (writing={} queueSize={})", writing_, writeQueue_.size());

    // Cancel pending async operations on the old stream before destroying it.
    // This causes in-flight async_read/async_write to complete with operation_aborted.
    beast::error_code ec;
    beast::get_lowest_layer(*ws_).socket().close(ec);

    // Clear all pending state from previous connection
    pendingPong_ = false;
    writing_ = false;
    readBuf_.consume(readBuf_.size());
    while (!writeQueue_.empty()) {
        writeQueue_.pop();
    }

    ws_ = std::make_unique<websocket::stream<beast::ssl_stream<beast::tcp_stream>>>(
        net::make_strand(ioc_), sslCtx_);
    doResolve();
}

void WebsocketRunner::setupControlCallback() {
    ws_->control_callback(
        [this](websocket::frame_type kind, beast::string_view) {
            if (kind == websocket::frame_type::pong) {
                lastPongTime_ = std::chrono::steady_clock::now();
                pendingPong_ = false;
            }
        });
}

void WebsocketRunner::startPingTimer() {
    if (stopping_ || !connected_) return;

    pingTimer_ = std::make_unique<net::steady_timer>(ioc_, std::chrono::seconds(PING_INTERVAL_SECS));
    pingTimer_->async_wait([this](const boost::system::error_code& ec) {
        if (!ec && !stopping_ && connected_) {
            doPing();
        }
    });
}

void WebsocketRunner::doPing() {
    if (stopping_ || !connected_) return;

    if (pendingPong_) {
        auto elapsed = std::chrono::steady_clock::now() - lastPongTime_;
        if (elapsed > std::chrono::seconds(PONG_TIMEOUT_SECS)) {
            getLogger()->error("Pong timeout, reconnecting...");
            connected_ = false;
            listener_.onWsDisconnected(true, "Pong timeout");
            scheduleReconnect();
            return;
        }
    }

    pendingPong_ = true;
    std::string pingMsg = R"({"method":"ping"})";
    net::post(ws_->get_executor(), [this, pingMsg]() {
        writeQueue_.push(pingMsg);
        if (connected_ && !writing_) {
            doWrite();
        }
        startPingTimer();
    });
}

}
}
