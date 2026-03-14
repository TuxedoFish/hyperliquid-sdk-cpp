#pragma once
#include <cstdint>
#include <iomanip>
#include <optional>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

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
        // Info endpoints (Perpetuals)
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
        PerpDexs,
        // Info endpoints (Spot)
        SpotMeta,
        SpotMetaAndAssetCtxs,
        SpotClearinghouseState,
        SpotDeployState,
        SpotPairDeployAuctionStatus,
        TokenDetails,

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
        case RestEndpointType::PerpDexs: return "perpDexs";

        case RestEndpointType::SpotMeta: return "spotMeta";
        case RestEndpointType::SpotMetaAndAssetCtxs: return "spotMetaAndAssetCtxs";
        case RestEndpointType::SpotClearinghouseState: return "spotClearinghouseState";
        case RestEndpointType::SpotDeployState: return "spotDeployState";
        case RestEndpointType::SpotPairDeployAuctionStatus: return "spotPairDeployAuctionStatus";
        case RestEndpointType::TokenDetails: return "tokenDetails";

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
        case RestEndpointType::PerpDexs: return false;

        case RestEndpointType::SpotMeta: return false;
        case RestEndpointType::SpotMetaAndAssetCtxs: return false;
        case RestEndpointType::SpotClearinghouseState: return false;
        case RestEndpointType::SpotDeployState: return false;
        case RestEndpointType::SpotPairDeployAuctionStatus: return false;
        case RestEndpointType::TokenDetails: return false;

        case RestEndpointType::PlaceOrder: return true;
        case RestEndpointType::CancelOrder: return true;
        case RestEndpointType::CancelOrderByCloid: return true;
        case RestEndpointType::ScheduleCancel: return true;
        case RestEndpointType::ModifyOrder: return true;
        case RestEndpointType::BatchModifyOrder: return true;
        default: throw std::invalid_argument("Unknown RestEndpointType");
        }
    }

    inline std::string toPath(RestEndpointType type)
    {
        return isAuthenticated(type) ? "/exchange" : "/info";
    }

    enum class Tif { Alo, Ioc, Gtc };

    inline std::string toString(Tif tif)
    {
        switch (tif)
        {
        case Tif::Alo: return "Alo";
        case Tif::Ioc: return "Ioc";
        case Tif::Gtc: return "Gtc";
        default: throw std::invalid_argument("Unknown Tif");
        }
    }

    enum class TpSl { Tp, Sl };

    inline std::string toString(TpSl tpsl)
    {
        switch (tpsl)
        {
        case TpSl::Tp: return "tp";
        case TpSl::Sl: return "sl";
        default: throw std::invalid_argument("Unknown TpSl");
        }
    }

    struct LimitOrderType
    {
        Tif tif;
    };

    struct TriggerOrderType
    {
        bool isMarket;
        double triggerPx;
        TpSl tpsl;
    };

    struct OrderRequest
    {
        std::string asset;
        bool isBuy;
        double price;
        double size;
        bool reduceOnly;
        std::optional<LimitOrderType> limit;
        std::optional<TriggerOrderType> trigger;
        std::optional<std::string> cloid;
    };

    enum class Grouping { Na, NormalTpsl, PositionTpsl };

    inline std::string toString(Grouping grouping)
    {
        switch (grouping)
        {
        case Grouping::Na: return "na";
        case Grouping::NormalTpsl: return "normalTpsl";
        case Grouping::PositionTpsl: return "positionTpsl";
        default: throw std::invalid_argument("Unknown Grouping");
        }
    }

    struct CancelRequest
    {
        std::string asset;
        uint64_t oid;
    };

    struct CancelByCloidRequest
    {
        std::string asset;
        std::string cloid;
    };

    struct ModifyRequest
    {
        std::optional<uint64_t> oid;
        std::optional<std::string> cloid;
        OrderRequest order;
    };

    struct Builder
    {
        std::string address;
        int fee;
    };

    inline std::string generateCloid()
    {
        std::random_device rd;
        std::mt19937_64 gen(rd());
        std::uniform_int_distribution<uint64_t> dist;

        uint64_t hi = dist(gen);
        uint64_t lo = dist(gen);

        std::ostringstream oss;
        oss << "0x"
            << std::hex << std::setfill('0')
            << std::setw(16) << hi
            << std::setw(16) << lo;
        return oss.str();
    }
}
