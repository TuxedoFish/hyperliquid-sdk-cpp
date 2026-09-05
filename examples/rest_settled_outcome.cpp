#include <hyperliquid/rest/RestApi.h>
#include <hyperliquid/config/Config.h>
#include <spdlog/spdlog.h>

int main() {
    hyperliquid::setLogLevel(hyperliquid::LogLevel::Debug);

    hyperliquid::ApiConfig config;
    config.env = hyperliquid::Environment::Testnet;

    hyperliquid::RestApi api(config);

    // Sad path: outcome ids are allocated sequentially and 1 predates any real market -
    // the endpoint returns a JSON null for an outcome that was never settled.
    spdlog::info("=== settledOutcome(1) - never settled ===");
    auto missing = api.settledOutcome(1);
    spdlog::info("  isSettled={}", missing.isSettled);

    // Happy path: a settled outcome with no linked question (a standalone recurring market).
    spdlog::info("=== settledOutcome(1000) - settled, no question ===");
    auto plain = api.settledOutcome(1000);
    spdlog::info("  isSettled={} spec.name={} settleFraction={} details={} hasQuestion={}",
                 plain.isSettled, plain.spec.name, plain.settleFraction, plain.details,
                 plain.question.has_value());

    // Happy path: a settled outcome whose linked question has itself already settled.
    spdlog::info("=== settledOutcome(10100) - settled, question settled ===");
    auto questionSettled = api.settledOutcome(10100);
    if (questionSettled.question) {
        spdlog::info("  isSettled={} question.isSettled={} question.questionId={} question.name={}",
                     questionSettled.isSettled, questionSettled.question->isSettled,
                     questionSettled.question->questionId, questionSettled.question->name);
    }

    // Happy path: a settled outcome whose linked question is still active (e.g. a multi-outcome
    // event like a tournament winner, where individual contestants settle to "No" as they're
    // eliminated while the overall question stays open).
    spdlog::info("=== settledOutcome(10234) - settled, question still active ===");
    auto questionActive = api.settledOutcome(10234);
    if (questionActive.question) {
        spdlog::info("  isSettled={} question.isSettled={} question.questionId={} question.name={}",
                     questionActive.isSettled, questionActive.question->isSettled,
                     questionActive.question->questionId, questionActive.question->name);
    }

    return 0;
}
