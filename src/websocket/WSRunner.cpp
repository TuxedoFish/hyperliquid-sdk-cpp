#include "WSRunner.h"
#include <iostream>
#include <algorithm>

namespace hyperliquid {
namespace internal {

WSRunner::WSRunner(const std::string& host, const std::string& port, const std::string& path, WSListener& listener)
    : listener_(listener)
    , sslCtx_(ssl::context::tlsv12_client)
    , resolver_(net::make_strand(ioc_))
    , ws_(std::make_unique<websocket::stream<beast::ssl_stream<beast::tcp_stream>>>(net::make_strand(ioc_), sslCtx_))
    , host_(host)
    , port_(port)
    , path_(path) {

    sslCtx_.set_default_verify_paths();
    sslCtx_.set_verify_mode(ssl::verify_peer);
}

WSRunner::~WSRunner() {
    stop();
}

net::io_context& WSRunner::getIoContext() { return ioc_; }

void WSRunner::send(const std::string& message) {
    net::post(ws_->get_executor(), [this, msg = message]() {
        writeQueue_.push(msg);
        if (connected_ && !writing_) {
            doWrite();
        }
    });
}

void WSRunner::start() {
    doResolve();
    ioc_.run();
}

void WSRunner::stop() {
    if (reconnectTimer_) reconnectTimer_->cancel();
    if (pingTimer_) pingTimer_->cancel();
    if (!ioc_.stopped() && !stopping_) {
        net::post(ws_->get_executor(), [this]() { doClose(); });
    }
}

void WSRunner::doResolve() {
    resolver_.async_resolve(host_, port_,
        [this](beast::error_code ec, tcp::resolver::results_type results) {
            onResolve(ec, results);
        });
}

void WSRunner::onResolve(beast::error_code ec, tcp::resolver::results_type results) {
    if (ec) {
        std::cerr << "resolve error: " << ec.message() << std::endl;
        scheduleReconnect();
        return;
    }

    beast::get_lowest_layer(*ws_).expires_after(std::chrono::seconds(10));

    beast::get_lowest_layer(*ws_).async_connect(results,
        [this](beast::error_code ec, tcp::resolver::results_type::endpoint_type ep) {
            onConnect(ec, ep);
        });
}

void WSRunner::onConnect(beast::error_code ec, tcp::resolver::results_type::endpoint_type) {
    if (ec) {
        std::cerr << "connect error: " << ec.message() << std::endl;
        scheduleReconnect();
        return;
    }

    if (!SSL_set_tlsext_host_name(ws_->next_layer().native_handle(), host_.c_str())) {
        ec = beast::error_code(static_cast<int>(::ERR_get_error()), net::error::get_ssl_category());
        std::cerr << "SNI error: " << ec.message() << std::endl;
        scheduleReconnect();
        return;
    }

    beast::get_lowest_layer(*ws_).expires_after(std::chrono::seconds(10));

    ws_->next_layer().async_handshake(ssl::stream_base::client,
        [this](beast::error_code ec) { onSslHandshake(ec); });
}

void WSRunner::onSslHandshake(beast::error_code ec) {
    if (ec) {
        std::cerr << "SSL handshake error: " << ec.message() << std::endl;
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

void WSRunner::onWsHandshake(beast::error_code ec) {
    if (ec) {
        std::cerr << "WebSocket handshake error: " << ec.message() << std::endl;
        scheduleReconnect();
        return;
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

void WSRunner::doRead() {
    ws_->async_read(readBuf_,
        [this](beast::error_code ec, std::size_t bytesTransferred) {
            onRead(ec, bytesTransferred);
        });
}

void WSRunner::onRead(beast::error_code errorCode, std::size_t) {
    if (errorCode) {
        if (stopping_) {
            // read cancelled, now safe to close
            std::cout << "Closing websocket..." << std::endl;
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
            std::cerr << "Unexpected websocket close" << std::endl;
            listener_.onWsDisconnected(true, errorCode.message());
            connected_ = false;
            scheduleReconnect();
        } else {
            std::cerr << "Unexpected error: " << errorCode.message() << std::endl;
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

void WSRunner::doWrite() {
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

void WSRunner::onWrite(beast::error_code ec, std::size_t) {
    if (ec) {
        std::cerr << "write error: " << ec.message() << std::endl;
        return;
    }

    writeQueue_.pop();
    doWrite();
}

void WSRunner::doClose() {
    if (!connected_ || stopping_) return;
    if (pingTimer_) pingTimer_->cancel();
    stopping_ = true;
    std::cout << "Waiting to close websocket..." << std::endl;
    beast::get_lowest_layer(*ws_).cancel();
}

void WSRunner::scheduleReconnect() {
    if (stopping_) return;

    reconnectAttempts_++;
    int delaySecs = std::min(1 << reconnectAttempts_, MAX_BACKOFF_SECS);
    std::cout << "Reconnecting in " << delaySecs << "s (attempt " << reconnectAttempts_ << ")" << std::endl;

    reconnectTimer_ = std::make_unique<net::steady_timer>(ioc_, std::chrono::seconds(delaySecs));
    reconnectTimer_->async_wait([this](const boost::system::error_code& ec) {
        if (!ec && !stopping_) {
            doReconnect();
        }
    });
}

void WSRunner::doReconnect() {
    pendingPong_ = false;
    ws_ = std::make_unique<websocket::stream<beast::ssl_stream<beast::tcp_stream>>>(
        net::make_strand(ioc_), sslCtx_);
    doResolve();
}

void WSRunner::setupControlCallback() {
    ws_->control_callback(
        [this](websocket::frame_type kind, beast::string_view) {
            if (kind == websocket::frame_type::pong) {
                lastPongTime_ = std::chrono::steady_clock::now();
                pendingPong_ = false;
            }
        });
}

void WSRunner::startPingTimer() {
    if (stopping_ || !connected_) return;

    pingTimer_ = std::make_unique<net::steady_timer>(ioc_, std::chrono::seconds(PING_INTERVAL_SECS));
    pingTimer_->async_wait([this](const boost::system::error_code& ec) {
        if (!ec && !stopping_ && connected_) {
            doPing();
        }
    });
}

void WSRunner::doPing() {
    if (stopping_ || !connected_) return;

    if (pendingPong_) {
        auto elapsed = std::chrono::steady_clock::now() - lastPongTime_;
        if (elapsed > std::chrono::seconds(PONG_TIMEOUT_SECS)) {
            std::cerr << "Pong timeout, reconnecting..." << std::endl;
            connected_ = false;
            listener_.onWsDisconnected(true, "Pong timeout");
            scheduleReconnect();
            return;
        }
    }

    pendingPong_ = true;
    ws_->async_ping({}, [this](beast::error_code ec) {
        if (ec) {
            if (!stopping_) {
                std::cerr << "Ping failed: " << ec.message() << std::endl;
                connected_ = false;
                listener_.onWsDisconnected(true, "Ping failed");
                scheduleReconnect();
            }
            return;
        }
        startPingTimer();
    });
}

}
}
