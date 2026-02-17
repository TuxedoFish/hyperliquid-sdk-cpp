#include <hyperliquid/rest/InfoApi.h>
#include <hyperliquid/rest/RestListener.h>
#include <hyperliquid/rest/RestMessageParser.h>
#include <hyperliquid/rest/InfoEndpointListener.h>
#include <hyperliquid/types/InfoEndpointTypes.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>

class MetaPrinter : public hyperliquid::RestListener, public hyperliquid::InfoEndpointListener {
public:
    std::atomic<bool> done{false};

    // hyperliquid::RestListener — raw message arrives here
    void onMessage(const std::string& message, hyperliquid::InfoEndpointType type) override {
        hyperliquid::RestMessageParser parser(*this);
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
    hyperliquid::InfoApi api(hyperliquid::Environment::Mainnet, printer);

    std::cout << "Fetching meta from Hyperliquid... (dex=xyz)" << std::endl;
    api.sendRequest(hyperliquid::InfoEndpointType::Meta, {{"dex", "xyz"}});

    // Wait for the async response
    while (!printer.done) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return 0;
}
