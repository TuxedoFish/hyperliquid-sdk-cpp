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

    // hyperliquid::RestListener — raw message arrives here
    void onMessage(const std::string& message, hyperliquid::RestEndpointType type) override {
        hyperliquid::RestApiMessageParser parser(*this);
        parser.parse(message, type);
    }

    // hyperliquid::InfoEndpointListener — parsed response arrives here
    void onMeta(const hyperliquid::MetaResponse& response) override {
        std::cout << "Universe: " << response.universe.size() << " assets" << std::endl;
        std::cout << std::endl;
        for (const auto& asset : response.universe) {
            std::cout << asset.name
                      << "  szDecimals=" << asset.szDecimals
                      << "  maxLeverage=" << asset.maxLeverage
                      << std::endl;
        }
        done = true;
    }
};

int main()
{
    OrderSender printer;
    hyperliquid::Wallet wallet{"", ""};
    hyperliquid::RestApi api(hyperliquid::Environment::Mainnet, printer, wallet);

    std::cout << "Sending order to Hyperliquid..." << std::endl;
    api.sendRequest(hyperliquid::RestEndpointType::PlaceOrder, {
        {"orders", [

        ]}
    });

    // Wait for the async response
    while (!printer.done) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return 0;
}
