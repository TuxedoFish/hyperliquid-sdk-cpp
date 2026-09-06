#include <chrono>
#include <thread>

#include <spdlog/spdlog.h>

#include "test_config.h"
#include "hyperliquid/types/RequestTypes.h"
#include "hyperliquid/websocket/WebsocketApi.h"
#include "hyperliquid/websocket/WebsocketApiListener.h"

class PostResponseLogger : public hyperliquid::WebsocketApiListener
{
public:
    // Typed callbacks - demonstrate the new mechanism with real parsed fields instead of raw JSON.
    void onPerpAnnotationPostResponse(const hyperliquid::PerpAnnotationResponse& resp,
                                      std::optional<uint64_t> correlationId) override
    {
        spdlog::info("[perpAnnotation] correlationId={} category={} description={}",
                     correlationId.value_or(0), resp.category, resp.description);
    }

    void onPerpCategoriesPostResponse(const hyperliquid::PerpCategoriesResponse& resp,
                                      std::optional<uint64_t> correlationId) override
    {
        spdlog::info("[perpCategories] correlationId={} categories={}",
                     correlationId.value_or(0), resp.categories.size());
    }

    void onPerpConciseAnnotationsPostResponse(const hyperliquid::PerpConciseAnnotationsResponse& resp,
                                              std::optional<uint64_t> correlationId) override
    {
        spdlog::info("[perpConciseAnnotations] correlationId={} annotations={}",
                     correlationId.value_or(0), resp.annotations.size());
    }

    void onAllPerpMetasPostResponse(const hyperliquid::AllPerpMetasResponse& resp,
                                    std::optional<uint64_t> correlationId) override
    {
        spdlog::info("[allPerpMetas] correlationId={} dexMetas={}",
                     correlationId.value_or(0), resp.dexMetas.size());
    }

    void onPerpsAtOpenInterestCapPostResponse(const hyperliquid::PerpsAtOpenInterestCapResponse& resp,
                                              std::optional<uint64_t> correlationId) override
    {
        spdlog::info("[perpsAtOpenInterestCap] correlationId={} coins={}",
                     correlationId.value_or(0), resp.coins.size());
    }

    void onPredictedFundingsPostResponse(const hyperliquid::PredictedFundingsResponse& resp,
                                         std::optional<uint64_t> correlationId) override
    {
        spdlog::info("[predictedFundings] correlationId={} fundings={}",
                     correlationId.value_or(0), resp.fundings.size());
    }

    // Generic fallback - still fires for every post response, typed or not.
    void onPostResponse(const std::string& rawJson, hyperliquid::RestEndpointType type,
                        std::optional<uint64_t> correlationId) override
    {
        spdlog::debug("[post response] type={} correlationId={} payload={}",
                     hyperliquid::toString(type), correlationId.value_or(0), rawJson);
    }

    void onConnected() override
    {
        spdlog::info("Connected");
    }

    void onDisconnected(bool hasError, const std::string& errMsg) override
    {
        spdlog::info("Disconnected: hasError={} errMsg={}", hasError, errMsg);
    }
};

int main()
{
    std::string userAddress = loadWalletFromConfig().accountAddress;
    hyperliquid::setLogLevel(hyperliquid::LogLevel::Info);

    hyperliquid::ApiConfig config;
    config.env = hyperliquid::Environment::Testnet;
    config.skipBuildingSymbolMap = true;

    PostResponseLogger logger;
    hyperliquid::WebsocketApi ws(config, logger);

    ws.start();
    std::this_thread::sleep_for(std::chrono::seconds(2));


    uint64_t correlationId = 1;
    auto wait = []() { std::this_thread::sleep_for(std::chrono::milliseconds(300)); };

    ws.perpAnnotation("BTC", correlationId++); wait();
    ws.perpCategories(correlationId++); wait();
    ws.perpConciseAnnotations(correlationId++); wait();
    ws.allPerpMetas(correlationId++); wait();
    ws.perpsAtOpenInterestCap(std::nullopt, correlationId++); wait();
    ws.predictedFundings(correlationId++); wait();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    ws.stop();
    return 0;
}
