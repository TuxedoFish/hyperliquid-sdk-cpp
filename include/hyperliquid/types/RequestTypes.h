#pragma once
#include <cstdint>
#include <iomanip>
#include <optional>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <variant>
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
        SpotState,
        AllDexsClearinghouseState,
        AllDexsAssetCtxs,
        FastAssetCtxs,
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
        case SubscriptionType::ClearingHouseState: return "clearinghouseState";
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
        case SubscriptionType::SpotState: return "spotState";
        case SubscriptionType::AllDexsClearinghouseState: return "allDexsClearinghouseState";
        case SubscriptionType::AllDexsAssetCtxs: return "allDexsAssetCtxs";
        case SubscriptionType::FastAssetCtxs: return "fastAssetCtxs";
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
        if (type == "clearinghouseState") return SubscriptionType::ClearingHouseState;
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
        if (type == "spotState") return SubscriptionType::SpotState;
        if (type == "allDexsClearinghouseState") return SubscriptionType::AllDexsClearinghouseState;
        if (type == "allDexsAssetCtxs") return SubscriptionType::AllDexsAssetCtxs;
        if (type == "fastAssetCtxs") return SubscriptionType::FastAssetCtxs;
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
        ClearinghouseState,
        UserRateLimit,
        PerpDexs,
        FrontendOpenOrders,
        HistoricalOrders,
        UserTwapSliceFills,
        SubAccounts,
        UserFees,
        MaxBuilderFee,
        ApprovedBuilders,
        VaultDetails,
        UserVaultEquities,
        Portfolio,
        Referral,
        UserRole,
        PerpsAtOpenInterestCap,
        PredictedFundings,
        PerpAnnotation,
        PerpCategories,
        PerpConciseAnnotations,
        AllPerpMetas,
        // Info endpoints (Outcomes)
        OutcomeMeta,
        // Info endpoints (Spot)
        SpotMeta,
        SpotMetaAndAssetCtxs,
        SpotClearinghouseState,
        SpotDeployState,
        SpotPairDeployAuctionStatus,
        TokenDetails,
        // Info endpoints (Staking)
        Delegations,
        DelegatorSummary,
        DelegatorHistory,
        DelegatorRewards,

        // Exchange endpoints (signed, L1 action)
        PlaceOrder,
        CancelOrder,
        CancelOrderByCloid,
        ScheduleCancel,
        ModifyOrder,
        BatchModifyOrder,
        UpdateLeverage,
        UpdateIsolatedMargin,
        ApproveAgent,
        TwapOrder,
        TwapCancel,
        VaultTransfer,
        UsdClassTransfer,
        SendAsset,
        UsdSend,
        SpotSend,
        Withdraw3,
        ApproveBuilderFee,

        // Exchange endpoints (signed, user-signed action)
        CDeposit,
        CWithdraw,
        TokenDelegate,
        SendToEvmWithData,
        AgentSendAsset,
        ReserveRequestWeight,
        Noop,
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
        case RestEndpointType::ClearinghouseState: return "clearinghouseState";
        case RestEndpointType::UserRateLimit: return "userRateLimit";
        case RestEndpointType::PerpDexs: return "perpDexs";
        case RestEndpointType::FrontendOpenOrders: return "frontendOpenOrders";
        case RestEndpointType::HistoricalOrders: return "historicalOrders";
        case RestEndpointType::UserTwapSliceFills: return "userTwapSliceFills";
        case RestEndpointType::SubAccounts: return "subAccounts";
        case RestEndpointType::UserFees: return "userFees";
        case RestEndpointType::MaxBuilderFee: return "maxBuilderFee";
        case RestEndpointType::ApprovedBuilders: return "approvedBuilders";
        case RestEndpointType::VaultDetails: return "vaultDetails";
        case RestEndpointType::UserVaultEquities: return "userVaultEquities";
        case RestEndpointType::Portfolio: return "portfolio";
        case RestEndpointType::Referral: return "referral";
        case RestEndpointType::UserRole: return "userRole";
        case RestEndpointType::PerpsAtOpenInterestCap: return "perpsAtOpenInterestCap";
        case RestEndpointType::PredictedFundings: return "predictedFundings";
        case RestEndpointType::PerpAnnotation: return "perpAnnotation";
        case RestEndpointType::PerpCategories: return "perpCategories";
        case RestEndpointType::PerpConciseAnnotations: return "perpConciseAnnotations";
        case RestEndpointType::AllPerpMetas: return "allPerpMetas";

        case RestEndpointType::OutcomeMeta: return "outcomeMeta";

        case RestEndpointType::SpotMeta: return "spotMeta";
        case RestEndpointType::SpotMetaAndAssetCtxs: return "spotMetaAndAssetCtxs";
        case RestEndpointType::SpotClearinghouseState: return "spotClearinghouseState";
        case RestEndpointType::SpotDeployState: return "spotDeployState";
        case RestEndpointType::SpotPairDeployAuctionStatus: return "spotPairDeployAuctionStatus";
        case RestEndpointType::TokenDetails: return "tokenDetails";

        case RestEndpointType::Delegations: return "delegations";
        case RestEndpointType::DelegatorSummary: return "delegatorSummary";
        case RestEndpointType::DelegatorHistory: return "delegatorHistory";
        case RestEndpointType::DelegatorRewards: return "delegatorRewards";

        case RestEndpointType::PlaceOrder: return "order";
        case RestEndpointType::CancelOrder: return "cancel";
        case RestEndpointType::CancelOrderByCloid: return "cancelByCloid";
        case RestEndpointType::ScheduleCancel: return "scheduleCancel";
        case RestEndpointType::ModifyOrder: return "modify";
        case RestEndpointType::BatchModifyOrder: return "batchModify";
        case RestEndpointType::UpdateLeverage: return "updateLeverage";
        case RestEndpointType::UpdateIsolatedMargin: return "updateIsolatedMargin";
        case RestEndpointType::ApproveAgent: return "approveAgent";
        case RestEndpointType::TwapOrder: return "twapOrder";
        case RestEndpointType::TwapCancel: return "twapCancel";
        case RestEndpointType::VaultTransfer: return "vaultTransfer";
        case RestEndpointType::UsdClassTransfer: return "usdClassTransfer";
        case RestEndpointType::SendAsset: return "sendAsset";
        case RestEndpointType::UsdSend: return "usdSend";
        case RestEndpointType::SpotSend: return "spotSend";
        case RestEndpointType::Withdraw3: return "withdraw3";
        case RestEndpointType::ApproveBuilderFee: return "approveBuilderFee";

        case RestEndpointType::CDeposit: return "cDeposit";
        case RestEndpointType::CWithdraw: return "cWithdraw";
        case RestEndpointType::TokenDelegate: return "tokenDelegate";
        case RestEndpointType::SendToEvmWithData: return "sendToEvmWithData";
        case RestEndpointType::AgentSendAsset: return "agentSendAsset";
        case RestEndpointType::ReserveRequestWeight: return "reserveRequestWeight";
        case RestEndpointType::Noop: return "noop";
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
        case RestEndpointType::ClearinghouseState: return false;
        case RestEndpointType::UserRateLimit: return false;
        case RestEndpointType::PerpDexs: return false;
        case RestEndpointType::FrontendOpenOrders: return false;
        case RestEndpointType::HistoricalOrders: return false;
        case RestEndpointType::UserTwapSliceFills: return false;
        case RestEndpointType::SubAccounts: return false;
        case RestEndpointType::UserFees: return false;
        case RestEndpointType::MaxBuilderFee: return false;
        case RestEndpointType::ApprovedBuilders: return false;
        case RestEndpointType::VaultDetails: return false;
        case RestEndpointType::UserVaultEquities: return false;
        case RestEndpointType::Portfolio: return false;
        case RestEndpointType::Referral: return false;
        case RestEndpointType::UserRole: return false;
        case RestEndpointType::PerpsAtOpenInterestCap: return false;
        case RestEndpointType::PredictedFundings: return false;
        case RestEndpointType::PerpAnnotation: return false;
        case RestEndpointType::PerpCategories: return false;
        case RestEndpointType::PerpConciseAnnotations: return false;
        case RestEndpointType::AllPerpMetas: return false;

        case RestEndpointType::OutcomeMeta: return false;

        case RestEndpointType::SpotMeta: return false;
        case RestEndpointType::SpotMetaAndAssetCtxs: return false;
        case RestEndpointType::SpotClearinghouseState: return false;
        case RestEndpointType::SpotDeployState: return false;
        case RestEndpointType::SpotPairDeployAuctionStatus: return false;
        case RestEndpointType::TokenDetails: return false;

        case RestEndpointType::Delegations: return false;
        case RestEndpointType::DelegatorSummary: return false;
        case RestEndpointType::DelegatorHistory: return false;
        case RestEndpointType::DelegatorRewards: return false;

        case RestEndpointType::PlaceOrder: return true;
        case RestEndpointType::CancelOrder: return true;
        case RestEndpointType::CancelOrderByCloid: return true;
        case RestEndpointType::ScheduleCancel: return true;
        case RestEndpointType::ModifyOrder: return true;
        case RestEndpointType::BatchModifyOrder: return true;
        case RestEndpointType::UpdateLeverage: return true;
        case RestEndpointType::UpdateIsolatedMargin: return true;
        case RestEndpointType::ApproveAgent: return true;
        case RestEndpointType::TwapOrder: return true;
        case RestEndpointType::TwapCancel: return true;
        case RestEndpointType::VaultTransfer: return true;
        case RestEndpointType::UsdClassTransfer: return true;
        case RestEndpointType::SendAsset: return true;
        case RestEndpointType::UsdSend: return true;
        case RestEndpointType::SpotSend: return true;
        case RestEndpointType::Withdraw3: return true;
        case RestEndpointType::ApproveBuilderFee: return true;

        case RestEndpointType::CDeposit: return true;
        case RestEndpointType::CWithdraw: return true;
        case RestEndpointType::TokenDelegate: return true;
        case RestEndpointType::SendToEvmWithData: return true;
        case RestEndpointType::AgentSendAsset: return true;
        case RestEndpointType::ReserveRequestWeight: return true;
        case RestEndpointType::Noop: return true;
        default: throw std::invalid_argument("Unknown RestEndpointType");
        }
    }

    inline std::string toPath(RestEndpointType type)
    {
        return isAuthenticated(type) ? "/exchange" : "/info";
    }

    // usdClassTransfer/sendAsset/usdSend/spotSend/withdraw3/approveBuilderFee, the staking
    // actions (cDeposit/cWithdraw/tokenDelegate), and sendToEvmWithData are EIP-712 user-signed
    // actions (see Signing::prepareUserSignedActionBody), not L1 actions. All other authenticated
    // actions here (including agentSendAsset/reserveRequestWeight/noop) are L1 actions signed
    // with the agent/master key directly.
    inline bool isUserSignedAction(RestEndpointType type)
    {
        switch (type)
        {
        case RestEndpointType::UsdClassTransfer:
        case RestEndpointType::SendAsset:
        case RestEndpointType::UsdSend:
        case RestEndpointType::SpotSend:
        case RestEndpointType::Withdraw3:
        case RestEndpointType::ApproveBuilderFee:
        case RestEndpointType::CDeposit:
        case RestEndpointType::CWithdraw:
        case RestEndpointType::TokenDelegate:
        case RestEndpointType::SendToEvmWithData:
            return true;
        default:
            return false;
        }
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
        std::optional<int> assetId;
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
        std::optional<int> assetId;
    };

    struct CancelByCloidRequest
    {
        std::string asset;
        std::string cloid;
        std::optional<int> assetId;
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

    using OrderId = std::variant<uint64_t, std::string>;

    struct UpdateLeverageRequest
    {
        std::string asset;
        bool isCross;
        int leverage;
        std::optional<int> assetId;
    };

    struct UpdateIsolatedMarginRequest
    {
        std::string asset;
        bool isBuy;
        // Amount to add (positive) or remove (negative) from the isolated position's margin,
        // in USD with 6 decimals of precision (1,000,000 == 1 USD).
        int64_t ntli;
        std::optional<int> assetId;
    };

    struct ApproveAgentRequest
    {
        std::string agentAddress;
        std::optional<std::string> agentName;
    };

    struct TwapOrderRequest
    {
        std::string asset;
        bool isBuy;
        double size;
        bool reduceOnly;
        int minutes;
        bool randomize;
        std::optional<int> assetId;
    };

    struct TwapCancelRequest
    {
        std::string asset;
        uint64_t twapId;
        std::optional<int> assetId;
    };

    struct VaultTransferRequest
    {
        std::string vaultAddress;
        bool isDeposit;
        double usd;
    };

    struct UsdClassTransferRequest
    {
        double amount;
        bool toPerp;
    };

    struct SendAssetRequest
    {
        std::string destination;
        std::string sourceDex;
        std::string destinationDex;
        std::string token;
        double amount;
        std::string fromSubAccount;
    };

    struct UsdSendRequest
    {
        std::string destination;
        double amount;
    };

    struct SpotSendRequest
    {
        std::string destination;
        std::string token;
        double amount;
    };

    struct Withdraw3Request
    {
        std::string destination;
        double amount;
    };

    struct ApproveBuilderFeeRequest
    {
        std::string maxFeeRate;
        std::string builder;
    };

    struct TokenDelegateRequest
    {
        std::string validator;
        // Amount to delegate/undelegate, in wei (1 HYPE == 1e8 wei).
        uint64_t wei;
        bool isUndelegate;
    };

    enum class AddressEncoding { Hex, Base58 };

    inline std::string toString(AddressEncoding encoding)
    {
        switch (encoding)
        {
        case AddressEncoding::Hex: return "hex";
        case AddressEncoding::Base58: return "base58";
        default: throw std::invalid_argument("Unknown AddressEncoding");
        }
    }

    inline AddressEncoding stringToAddressEncoding(const std::string& encoding)
    {
        if (encoding == "hex") return AddressEncoding::Hex;
        if (encoding == "base58") return AddressEncoding::Base58;
        throw std::invalid_argument("Unknown AddressEncoding: " + encoding);
    }

    struct SendToEvmWithDataRequest
    {
        std::string token;
        std::string amount;
        std::string sourceDex;
        std::string destinationRecipient;
        AddressEncoding addressEncoding;
        uint32_t destinationChainId;
        uint64_t gasLimit;
        std::string data; // hex-encoded bytes, e.g. "0x..."
    };

    struct AgentSendAssetRequest
    {
        std::string destination;
        std::string sourceDex;
        std::string destinationDex;
        std::string token;
        std::string amount;
        std::optional<std::string> fromSubAccount;
    };

    struct ReserveRequestWeightRequest
    {
        int weight;
        std::optional<std::string> destination;
    };

    inline int outcomeEncoding(int outcomeIndex, int side)
    {
        return 10 * outcomeIndex + side;
    }

    inline int outcomeAssetId(int outcomeIndex, int side)
    {
        return 100000000 + outcomeEncoding(outcomeIndex, side);
    }

    inline std::string outcomeCoin(int outcomeIndex, int side)
    {
        return "#" + std::to_string(outcomeEncoding(outcomeIndex, side));
    }

    inline std::string outcomeToken(int outcomeIndex, int side)
    {
        return "+" + std::to_string(outcomeEncoding(outcomeIndex, side));
    }

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
