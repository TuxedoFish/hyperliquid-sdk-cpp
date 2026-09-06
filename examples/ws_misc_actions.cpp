#include <chrono>
#include <thread>

#include <spdlog/spdlog.h>

#include "test_config.h"
#include "hyperliquid/websocket/WebsocketApi.h"
#include "hyperliquid/websocket/WebsocketApiListener.h"

class PostResponseLogger : public hyperliquid::WebsocketApiListener
{
public:
    void onPostResponse(const std::string& rawJson, hyperliquid::RestEndpointType type,
                        std::optional<uint64_t> correlationId) override
    {
        spdlog::info("[post response] type={} correlationId={} payload={}",
                     hyperliquid::toString(type), correlationId.value_or(0), rawJson);
    }

    void onConnected() override
    {
        spdlog::info("Connected");
    }
};

int main()
{
    auto wallet = loadWalletFromConfig();
    hyperliquid::setLogLevel(hyperliquid::LogLevel::Debug);

    hyperliquid::ApiConfig config;
    config.env = hyperliquid::Environment::Testnet;
    config.wallet = wallet;

    PostResponseLogger logger;
    hyperliquid::WebsocketApi ws(config, logger);

    ws.start();
    std::this_thread::sleep_for(std::chrono::seconds(2));

    const std::string destination = "0x5e9ee1089755c3435139848e47e6635505d5a13a";
    const std::string tokenId = "USDC:0xeb62eee3685fc4c43992febcd9e75443";

    uint64_t correlationId = 1;
    auto wait = []() { std::this_thread::sleep_for(std::chrono::seconds(3)); };

    ws.noop(correlationId++); wait();

    hyperliquid::ReserveRequestWeightRequest reserveReq;
    reserveReq.weight = 10;
    ws.reserveRequestWeight(reserveReq, correlationId++); wait();

    hyperliquid::AgentSendAssetRequest agentSendReq;
    agentSendReq.destination = wallet.accountAddress; // agentSendAsset requires the account's own address
    agentSendReq.sourceDex = "";
    agentSendReq.destinationDex = "";
    agentSendReq.token = tokenId;
    agentSendReq.amount = "1";
    ws.agentSendAsset(agentSendReq, correlationId++); wait();

    hyperliquid::SendToEvmWithDataRequest evmReq;
    evmReq.token = tokenId;
    evmReq.amount = "1";
    evmReq.sourceDex = "";
    evmReq.destinationRecipient = destination;
    evmReq.addressEncoding = hyperliquid::AddressEncoding::Hex;
    evmReq.destinationChainId = 421614; // Arbitrum Sepolia testnet
    evmReq.gasLimit = 200000;
    evmReq.data = "0x";
    ws.sendToEvmWithData(evmReq, correlationId++); wait();
    ws.stop();
    return 0;
}
