#pragma once
#include <stdexcept>
#include <string>

namespace hyperliquid
{
    struct Endpoint
    {
        std::string host;
        std::string port;
        std::string path;
    };

    enum class Environment
    {
        Mainnet,
        Testnet
    };

    inline const Endpoint& toWsEndpoint(Environment env)
    {
        static const Endpoint mainnet{"api.hyperliquid.xyz", "443", "/ws"};
        static const Endpoint testnet{"api.hyperliquid-testnet.xyz", "443", "/ws"};
        switch (env)
        {
        case Environment::Mainnet: return mainnet;
        case Environment::Testnet: return testnet;
        default: throw std::invalid_argument("Unknown Environment");
        }
    }

    inline const Endpoint& toInfoEndpoint(Environment env)
    {
        static const Endpoint mainnet{"api.hyperliquid.xyz", "443", "/info"};
        static const Endpoint testnet{"api.hyperliquid-testnet.xyz", "443", "/info"};
        switch (env)
        {
        case Environment::Mainnet: return mainnet;
        case Environment::Testnet: return testnet;
        default: throw std::invalid_argument("Unknown Environment");
        }
    }

    // --- WebSocket subscription types ---

    enum class SubscriptionType
    {
        L2Book,
        Bbo,
        Trades,
        Candle,
        AllMids,
        Notification,
        WebData3,
        TwapStates,
        ClearingHouseState,
        OpenOrders,
        OrderUpdates,
        UserEvents,
        UserFills,
        UserFundings,
        UserNonFundingLedgerUpdates,
        ActiveAssetCtx,
        ActiveAssetData,
        UserTwapSliceFills,
        UserTwapHistory,
    };

    inline std::string toString(SubscriptionType type)
    {
        switch (type)
        {
        case SubscriptionType::L2Book: return "l2Book";
        case SubscriptionType::Bbo: return "bbo";
        case SubscriptionType::Trades: return "trades";
        case SubscriptionType::Candle: return "candle";
        case SubscriptionType::AllMids: return "allMids";
        case SubscriptionType::Notification: return "notification";
        case SubscriptionType::WebData3: return "webData3";
        case SubscriptionType::TwapStates: return "twapStates";
        case SubscriptionType::ClearingHouseState: return "clearingHouseState";
        case SubscriptionType::OpenOrders: return "openOrders";
        case SubscriptionType::OrderUpdates: return "orderUpdates";
        case SubscriptionType::UserEvents: return "userEvents";
        case SubscriptionType::UserFills: return "userFills";
        case SubscriptionType::UserFundings: return "userFundings";
        case SubscriptionType::UserNonFundingLedgerUpdates: return "userNonFundingLedgerUpdates";
        case SubscriptionType::ActiveAssetCtx: return "activeAssetCtx";
        case SubscriptionType::ActiveAssetData: return "activeAssetData";
        case SubscriptionType::UserTwapSliceFills: return "userTwapSliceFills";
        case SubscriptionType::UserTwapHistory: return "userTwapHistory";
        default: throw std::invalid_argument("Unknown SubscriptionType");
        }
    }

    // --- REST info endpoint types ---

    enum class InfoEndpointType
    {
        Meta,
        MetaAndAssetCtxs,
        AllMids,
        L2Book,
        CandleSnapshot,
        OpenOrders,
        UserFills,
        UserFillsByTime,
        OrderStatus,
        UserRateLimit,
    };

    inline std::string toString(InfoEndpointType type)
    {
        switch (type)
        {
        case InfoEndpointType::Meta: return "meta";
        case InfoEndpointType::MetaAndAssetCtxs: return "metaAndAssetCtxs";
        case InfoEndpointType::AllMids: return "allMids";
        case InfoEndpointType::L2Book: return "l2Book";
        case InfoEndpointType::CandleSnapshot: return "candleSnapshot";
        case InfoEndpointType::OpenOrders: return "openOrders";
        case InfoEndpointType::UserFills: return "userFills";
        case InfoEndpointType::UserFillsByTime: return "userFillsByTime";
        case InfoEndpointType::OrderStatus: return "orderStatus";
        case InfoEndpointType::UserRateLimit: return "userRateLimit";
        default: throw std::invalid_argument("Unknown InfoEndpointType");
        }
    }
}
