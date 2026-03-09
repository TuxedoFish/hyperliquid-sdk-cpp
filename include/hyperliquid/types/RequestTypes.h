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
        Unknown,
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

    inline SubscriptionType stringToSubscriptionType(const std::string& type)
    {
        if (type == "l2Book") return SubscriptionType::L2Book;
        if (type == "bbo") return SubscriptionType::Bbo;
        if (type == "trades") return SubscriptionType::Trades;
        if (type == "candle") return SubscriptionType::Candle;
        if (type == "allMids") return SubscriptionType::AllMids;
        if (type == "notification") return SubscriptionType::Notification;
        if (type == "webData3") return SubscriptionType::WebData3;
        if (type == "twapStates") return SubscriptionType::TwapStates;
        if (type == "clearingHouseState") return SubscriptionType::ClearingHouseState;
        if (type == "openOrders") return SubscriptionType::OpenOrders;
        if (type == "orderUpdates") return SubscriptionType::OrderUpdates;
        if (type == "userEvents") return SubscriptionType::UserEvents;
        if (type == "userFills") return SubscriptionType::UserFills;
        if (type == "userFundings") return SubscriptionType::UserFundings;
        if (type == "userNonFundingLedgerUpdates") return SubscriptionType::UserNonFundingLedgerUpdates;
        if (type == "activeAssetCtx") return SubscriptionType::ActiveAssetCtx;
        if (type == "activeAssetData") return SubscriptionType::ActiveAssetData;
        if (type == "userTwapSliceFills") return SubscriptionType::UserTwapSliceFills;
        if (type == "userTwapHistory") return SubscriptionType::UserTwapHistory;
        return SubscriptionType::Unknown;
    }

    // --- Rest endpoint types ---

    enum class RestEndpointType
    {
        // Info endpoints
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

        // Exchange endpoints (signed)
        PlaceOrder,
        CancelOrder,
        CancelOrderByCloid,
        ScheduleCancel,
        ModifyOrder,
        BatchModifyOrder,
    };

    inline std::string toString(RestEndpointType type)
    {
        switch (type)
        {
        case RestEndpointType::Meta: return "meta";
        case RestEndpointType::MetaAndAssetCtxs: return "metaAndAssetCtxs";
        case RestEndpointType::AllMids: return "allMids";
        case RestEndpointType::L2Book: return "l2Book";
        case RestEndpointType::CandleSnapshot: return "candleSnapshot";
        case RestEndpointType::OpenOrders: return "openOrders";
        case RestEndpointType::UserFills: return "userFills";
        case RestEndpointType::UserFillsByTime: return "userFillsByTime";
        case RestEndpointType::OrderStatus: return "orderStatus";
        case RestEndpointType::UserRateLimit: return "userRateLimit";
        case RestEndpointType::PlaceOrder: return "order";
        case RestEndpointType::CancelOrder: return "cancel";
        case RestEndpointType::CancelOrderByCloid: return "cancelByCloid";
        case RestEndpointType::ScheduleCancel: return "scheduleCancel";
        case RestEndpointType::ModifyOrder: return "modify";
        case RestEndpointType::BatchModifyOrder: return "batchModify";
        default: throw std::invalid_argument("Unknown InfoEndpointType");
        }
    }

    inline bool isAuthenticated(RestEndpointType type)
    {
        switch (type)
        {
        case RestEndpointType::Meta: return false;
        case RestEndpointType::MetaAndAssetCtxs: return false;
        case RestEndpointType::AllMids: return false;
        case RestEndpointType::L2Book: return false;
        case RestEndpointType::CandleSnapshot: return false;
        case RestEndpointType::OpenOrders: return false;
        case RestEndpointType::UserFills: return false;
        case RestEndpointType::UserFillsByTime: return false;
        case RestEndpointType::OrderStatus: return false;
        case RestEndpointType::UserRateLimit: return false;
        case RestEndpointType::PlaceOrder: return true;
        case RestEndpointType::CancelOrder: return true;
        case RestEndpointType::CancelOrderByCloid: return true;
        case RestEndpointType::ScheduleCancel: return true;
        case RestEndpointType::ModifyOrder: return true;
        case RestEndpointType::BatchModifyOrder: return true;
        default: throw std::invalid_argument("Unknown RestEndpointType");
        }
    }
}
