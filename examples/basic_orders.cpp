#include "test_config.h"

#include <hyperliquid/rest/RestApi.h>
#include <hyperliquid/rest/RestApiListener.h>
#include <hyperliquid/rest/RestApiMessageParser.h>
#include <hyperliquid/rest/RestEndpointListener.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>

class OrderSender : public hyperliquid::RestApiListener, public hyperliquid::RestEndpointListener {
public:
    std::atomic<bool> done{false};

    void onMessage(const std::string& message, hyperliquid::RestEndpointType type) override {
        hyperliquid::RestApiMessageParser parser(*this);
        parser.parse(message, type);
    }

    void onPlaceOrder(const hyperliquid::PlaceOrderResponse& response) override {
        std::cout << "Order response status: " << response.status << std::endl;
        for (const auto& status : response.statuses) {
            if (status.resting) {
                std::cout << "  Resting oid=" << status.resting->oid << std::endl;
            } else if (status.filled) {
                std::cout << "  Filled oid=" << status.filled->oid
                          << " avgPx=" << status.filled->avgPx
                          << " totalSz=" << status.filled->totalSz << std::endl;
            } else if (status.error) {
                std::cout << "  Error: " << *status.error << std::endl;
            }
        }
        done = true;
    }
};

int main()
{
    OrderSender sender;
    auto wallet = loadWalletFromConfig();
    hyperliquid::RestApi api(hyperliquid::Environment::Testnet, sender, wallet);

    hyperliquid::OrderRequest order;
    order.asset = "BTC";
    order.isBuy = true;
    order.price = "1800.0";
    order.size = "0.01";
    order.reduceOnly = false;
    order.limit = hyperliquid::LimitOrderType{hyperliquid::Tif::Gtc};

    std::cout << "Placing order on Hyperliquid testnet..." << std::endl;
    api.placeOrderAsync({order}, hyperliquid::Grouping::Na);

    while (!sender.done) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return 0;
}
