#include <hyperliquid/rest/RestApi.h>
#include <hyperliquid/rest/RestApiListener.h>
#include <hyperliquid/rest/RestApiMessageParser.h>
#include <hyperliquid/rest/RestEndpointListener.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>

#include "hyperliquid/types/ResponseTypes.h"

class MetaPrinter : public hyperliquid::RestApiListener, public hyperliquid::RestEndpointListener {
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

int main() {
    MetaPrinter printer;
    hyperliquid::RestApi api(hyperliquid::Environment::Mainnet, printer);

    std::cout << "Fetching meta from Hyperliquid... (dex=xyz)" << std::endl;
    api.sendRequest(hyperliquid::RestEndpointType::Meta, {{"dex", "xyz"}});

    // Wait for the async response
    while (!printer.done) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return 0;
}
