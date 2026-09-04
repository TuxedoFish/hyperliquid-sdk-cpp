#include "test_config.h"

#include <chrono>

#include <hyperliquid/rest/RestApi.h>
#include <hyperliquid/config/Config.h>
#include <spdlog/spdlog.h>

void logSimpleResponse(const char* label, const hyperliquid::SimpleResponse& resp)
{
    if (resp.error)
        spdlog::info("{}: status={} error={}", label, resp.status, *resp.error);
    else
        spdlog::info("{}: status={}", label, resp.status);
}

int main()
{
    auto wallet = loadWalletFromConfig();
    hyperliquid::setLogLevel(hyperliquid::LogLevel::Debug);

    hyperliquid::ApiConfig config;
    config.env = hyperliquid::Environment::Testnet;
    config.wallet = wallet;

    hyperliquid::RestApi api(config);

    // Arm: schedule a cancel-all to fire 60 seconds from now (must be at
    // least 5 seconds in the future per the exchange's rules).
    uint64_t nowMs = static_cast<uint64_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count());
    uint64_t scheduledTimeMs = nowMs + 60'000;

    spdlog::info("Arming dead man's switch for time={}...", scheduledTimeMs);
    logSimpleResponse("Arm", api.scheduleCancel(scheduledTimeMs));

    // Disarm: calling scheduleCancel with no timestamp removes any
    // previously scheduled cancel-all.
    spdlog::info("Disarming dead man's switch...");
    logSimpleResponse("Disarm", api.scheduleCancel());

    return 0;
}
