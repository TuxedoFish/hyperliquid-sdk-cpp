#pragma once

#include <array>
#include <chrono>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "RequestTypes.h"

namespace hyperliquid
{

    // --- Subscription types ---

    enum class SubscriptionMethod { Subscribe, Unsubscribe };

    struct Subscription
    {
        SubscriptionType type;
    };

    struct SubscriptionResponse
    {
        SubscriptionMethod method;
        Subscription subscription;
    };

    // --- Websocket data types (unauthenticated) ---

    enum class Side { Bid, Ask };

    struct PriceLevel
    {
        Side side;
        std::string_view px;
        std::string_view sz;
        int n;
    };

    struct L2BookUpdate
    {
        std::string coin;
        uint64_t time;
    };

    static constexpr size_t L2_BOOK_MAX_LEVELS = 50;

    struct L2BookSnapshot
    {
        std::string coin;
        uint64_t time;
        std::array<PriceLevel, L2_BOOK_MAX_LEVELS> bids;
        std::array<PriceLevel, L2_BOOK_MAX_LEVELS> asks;
        uint8_t numBids;
        uint8_t numAsks;
    };

    struct BboUpdate
    {
        std::string coin;
        uint64_t time;
        bool hasBid;
        PriceLevel bid;
        bool hasAsk;
        PriceLevel ask;
    };

    struct Trade
    {
        std::string coin;
        char side;
        std::string_view px;
        std::string_view sz;
        uint64_t time;
        uint64_t tid;
        std::string hash;
        std::string buyer;
        std::string seller;
    };

    struct Candle
    {
        std::string coin;
        std::string interval;
        uint64_t openTime;
        uint64_t closeTime;
        double open;
        double close;
        double high;
        double low;
        double volume;
        int numTrades;
    };

    struct AllMidsEntry
    {
        std::string coin;
        double mid;
    };

    struct PerpAssetCtx
    {
        std::string coin;
        double dayNtlVlm;
        double prevDayPx;
        double markPx;
        double midPx;
        bool hasMidPx;
        double funding;
        double openInterest;
        double oraclePx;
    };

    struct SpotAssetCtx
    {
        std::string coin;
        double dayNtlVlm;
        double prevDayPx;
        double markPx;
        double midPx;
        bool hasMidPx;
        double circulatingSupply;
    };

    // --- Websocket data types (authenticated) ---

    enum class OrderStatus { Open, Filled, Canceled, Triggered, Rejected, MarginCanceled, OracleRejected, IocCancelRejected, Unknown };

    inline OrderStatus stringToOrderStatus(std::string_view s)
    {
        if (s == "open") return OrderStatus::Open;
        if (s == "filled") return OrderStatus::Filled;
        if (s == "canceled") return OrderStatus::Canceled;
        if (s == "triggered") return OrderStatus::Triggered;
        if (s == "rejected") return OrderStatus::Rejected;
        if (s == "marginCanceled") return OrderStatus::MarginCanceled;
        if (s == "oracleRejected") return OrderStatus::OracleRejected;
        if (s == "iocCancelRejected") return OrderStatus::IocCancelRejected;
        return OrderStatus::Unknown;
    }

    inline std::string toString(OrderStatus status)
    {
        switch (status)
        {
        case OrderStatus::Open: return "open";
        case OrderStatus::Filled: return "filled";
        case OrderStatus::Canceled: return "canceled";
        case OrderStatus::Triggered: return "triggered";
        case OrderStatus::Rejected: return "rejected";
        case OrderStatus::MarginCanceled: return "marginCanceled";
        case OrderStatus::OracleRejected: return "oracleRejected";
        case OrderStatus::IocCancelRejected: return "iocCancelRejected";
        default: return "unknown";
        }
    }

    enum class LiquidationMethod { Market, Backstop, Unknown };

    inline LiquidationMethod stringToLiquidationMethod(std::string_view s)
    {
        if (s == "market") return LiquidationMethod::Market;
        if (s == "backstop") return LiquidationMethod::Backstop;
        return LiquidationMethod::Unknown;
    }

    inline std::string toString(LiquidationMethod method)
    {
        switch (method)
        {
        case LiquidationMethod::Market: return "market";
        case LiquidationMethod::Backstop: return "backstop";
        default: return "unknown";
        }
    }

    struct Fill
    {
        std::string coin;
        double px;
        double sz;
        char side; // 'B' or 'A'
        uint64_t time;
        double startPosition;
        std::string dir;
        double closedPnl;
        std::string hash;
        uint64_t oid;
        bool crossed;
        double fee;
        uint64_t tid;
        std::string feeToken;
        double builderFee;
        bool hasBuilderFee;
        bool isLiquidation;
        std::string liquidatedUser;
        double liquidationMarkPx;
        LiquidationMethod liquidationMethod;
        bool isSnapshot;
    };

    struct OrderUpdate
    {
        std::string coin;
        char side;
        double limitPx;
        double sz;
        uint64_t oid;
        uint64_t timestamp;
        double origSz;
        std::string cloid;
        OrderStatus status;
        uint64_t statusTimestamp;
    };

    struct UserFunding
    {
        uint64_t time;
        std::string coin;
        double usdc;
        double szi;
        double fundingRate;
        bool isSnapshot = false;
    };

    struct Liquidation
    {
        uint64_t lid;
        std::string liquidator;
        std::string liquidatedUser;
        double liquidatedNtlPos;
        double liquidatedAccountValue;
    };

    struct NonUserCancel
    {
        std::string coin;
        uint64_t oid;
    };

    struct MarginSummary
    {
        double accountValue;
        double totalNtlPos;
        double totalRawUsd;
        double totalMarginUsed;
    };

    struct AssetPosition
    {
        std::string coin;
        double szi;
        double entryPx;
        double positionValue;
        double unrealizedPnl;
        double returnOnEquity;
        double liquidationPx;
        bool hasLiquidationPx;
        double marginUsed;
        int maxLeverage;
        std::string leverageType; // "Cross" or "Isolated"
    };

    struct ClearinghouseState
    {
        std::vector<AssetPosition> assetPositions;
        MarginSummary marginSummary;
        MarginSummary crossMarginSummary;
        double crossMaintenanceMarginUsed;
        double withdrawable;
        uint64_t time;
    };

    // Websocket `clearinghouseState` channel wraps the same InnerClearinghouseState
    // shape as the REST clearinghouseState endpoint, plus dex/user context.
    struct ClearinghouseStateUpdate
    {
        std::string dex;
        std::string user;
        ClearinghouseState state;
    };

    struct OpenOrder
    {
        std::string coin;
        char side;
        double limitPx;
        double sz;
        uint64_t oid;
        uint64_t timestamp;
        double origSz;
        std::string cloid;
    };

    // Websocket `openOrders` channel: a snapshot array of an account's resting orders.
    struct OpenOrdersUpdate
    {
        std::string dex;
        std::string user;
        std::vector<OpenOrder> orders;
    };

    struct TwapState
    {
        int id;
        std::string coin;
        std::string user;
        char side;
        double sz;
        double executedSz;
        double executedNtl;
        int minutes;
        bool reduceOnly;
        bool randomize;
        uint64_t timestamp;
    };

    struct TwapHistoryEntry
    {
        TwapState state;
        std::string status;
        std::string description;
        uint64_t time;
    };

    struct TwapSliceFill
    {
        Fill fill;
        int twapId;
    };

    enum class LedgerUpdateType
    {
        Deposit,
        Withdraw,
        InternalTransfer,
        SubAccountTransfer,
        Liquidation,
        VaultCreate,
        VaultDeposit,
        VaultDistribution,
        VaultWithdraw,
        VaultLeaderCommission,
        SpotTransfer,
        AccountClassTransfer,
        SpotGenesis,
        RewardsClaim,
        Unknown
    };

    inline LedgerUpdateType stringToLedgerUpdateType(std::string_view s)
    {
        if (s == "deposit") return LedgerUpdateType::Deposit;
        if (s == "withdraw") return LedgerUpdateType::Withdraw;
        if (s == "internalTransfer") return LedgerUpdateType::InternalTransfer;
        if (s == "subAccountTransfer") return LedgerUpdateType::SubAccountTransfer;
        if (s == "liquidation") return LedgerUpdateType::Liquidation;
        if (s == "vaultCreate") return LedgerUpdateType::VaultCreate;
        if (s == "vaultDeposit") return LedgerUpdateType::VaultDeposit;
        if (s == "vaultDistribution") return LedgerUpdateType::VaultDistribution;
        if (s == "vaultWithdraw") return LedgerUpdateType::VaultWithdraw;
        if (s == "vaultLeaderCommission") return LedgerUpdateType::VaultLeaderCommission;
        if (s == "spotTransfer") return LedgerUpdateType::SpotTransfer;
        if (s == "accountClassTransfer") return LedgerUpdateType::AccountClassTransfer;
        if (s == "spotGenesis") return LedgerUpdateType::SpotGenesis;
        if (s == "rewardsClaim") return LedgerUpdateType::RewardsClaim;
        return LedgerUpdateType::Unknown;
    }

    inline std::string toString(LedgerUpdateType type)
    {
        switch (type)
        {
        case LedgerUpdateType::Deposit: return "deposit";
        case LedgerUpdateType::Withdraw: return "withdraw";
        case LedgerUpdateType::InternalTransfer: return "internalTransfer";
        case LedgerUpdateType::SubAccountTransfer: return "subAccountTransfer";
        case LedgerUpdateType::Liquidation: return "liquidation";
        case LedgerUpdateType::VaultCreate: return "vaultCreate";
        case LedgerUpdateType::VaultDeposit: return "vaultDeposit";
        case LedgerUpdateType::VaultDistribution: return "vaultDistribution";
        case LedgerUpdateType::VaultWithdraw: return "vaultWithdraw";
        case LedgerUpdateType::VaultLeaderCommission: return "vaultLeaderCommission";
        case LedgerUpdateType::SpotTransfer: return "spotTransfer";
        case LedgerUpdateType::AccountClassTransfer: return "accountClassTransfer";
        case LedgerUpdateType::SpotGenesis: return "spotGenesis";
        case LedgerUpdateType::RewardsClaim: return "rewardsClaim";
        default: return "unknown";
        }
    }

    struct LiquidatedPosition
    {
        std::string coin;
        double szi;
    };

    // Flattened representation of the WsLedgerUpdate discriminated union
    // (deposit/withdraw/internalTransfer/.../rewardsClaim). `type` indicates
    // which delta this was; fields not applicable to that delta are left at
    // their default (zero/empty) value. See docs:
    // https://hyperliquid.gitbook.io/hyperliquid-docs/for-developers/api/websocket/subscriptions
    struct LedgerUpdate
    {
        uint64_t time;
        std::string hash;
        LedgerUpdateType type;
        double usdc;
        std::string user;
        std::string destination;
        std::string vault;
        std::string token;
        double fee;
        double amount;
        double usdcValue; // spotTransfer only
        uint64_t nonce;
        bool toPerp;
        // liquidation-only fields
        double accountValue;
        std::string leverageType; // "Cross" or "Isolated"
        std::vector<LiquidatedPosition> liquidatedPositions;
        // vaultWithdraw-only fields
        double requestedUsd;
        double commission;
        double closingCost;
        double basis;
        double netWithdrawnUsd;
        bool isSnapshot = false;
    };

    struct ActiveAssetData
    {
        std::string user;
        std::string coin;
        double maxTradeSzLong;
        double maxTradeSzShort;
        double availableToTradeLong;
        double availableToTradeShort;
    };

    struct Notification
    {
        std::string notification;
    };

    struct LeadingVault
    {
        std::string address;
        std::string name;
    };

    struct PerpDexState
    {
        double totalVaultEquity;
        std::vector<std::string> perpsAtOpenInterestCap;
        std::vector<LeadingVault> leadingVaults;
    };

    struct WebData3UserState
    {
        std::optional<std::string> agentAddress;
        std::optional<uint64_t> agentValidUntil;
        uint64_t serverTime;
        double cumLedger;
        bool isVault;
        std::string user;
        bool optOutOfSpotDusting;
        bool dexAbstractionEnabled;
    };

    // webData3 is a large, evolving "frontend" aggregate payload. Only the
    // documented, stable sub-fields (userState, perpDexStates) are modeled
    // as typed members; `raw` retains the full JSON of the `data` object as
    // an escape hatch so no information is silently dropped for the many
    // less-critical/undocumented sub-fields the docs note may change.
    struct WebData3Update
    {
        WebData3UserState userState;
        std::vector<PerpDexState> perpDexStates;
        std::string raw;
    };

    // --- Outcome market types ---

    struct OutcomeSideSpec
    {
        std::string name;
    };

    // Parses "20260503-0600" -> time_point (UTC)
    inline std::chrono::system_clock::time_point parseOutcomeExpiry(const std::string& s)
    {
        // Format: YYYYMMDD-HHMM
        std::tm tm = {};
        tm.tm_year = std::stoi(s.substr(0, 4)) - 1900;
        tm.tm_mon = std::stoi(s.substr(4, 2)) - 1;
        tm.tm_mday = std::stoi(s.substr(6, 2));
        tm.tm_hour = std::stoi(s.substr(9, 2));
        tm.tm_min = std::stoi(s.substr(11, 2));
        tm.tm_sec = 0;
        tm.tm_isdst = 0;
        std::time_t t = timegm(&tm);
        return std::chrono::system_clock::from_time_t(t);
    }

    struct OutcomeDescription
    {
        std::string outcomeClass;
        std::string underlying;
        std::chrono::system_clock::time_point expiry;
        std::string targetPrice;
        std::string period;
    };

    struct Outcome
    {
        int outcome;
        std::string name;
        std::string descriptionRaw;
        OutcomeDescription description;
        std::vector<OutcomeSideSpec> sideSpecs;
    };

    struct OutcomeMetaResponse
    {
        std::vector<Outcome> outcomes;
    };

    // --- Rest endpoint types (unauthenticated) ---

    struct AssetMeta
    {
        std::string name;
        int szDecimals;
        int maxLeverage;
    };

    struct MetaResponse
    {
        std::vector<AssetMeta> universe;
    };

    struct EvmContract
    {
        std::string address;
        int evm_extra_wei_decimals;
    };

    struct SpotAssetMeta
    {
        std::string name;
        int szDecimals;
        int weiDecimals;
        int index;
        std::string tokenId;
        bool isCanonical;
        std::optional<EvmContract> evmContract;
        std::optional<std::string> fullName;
    };

    struct SpotMetaResponse
    {
        std::vector<SpotAssetMeta> tokens;
    };

    struct PerpDex
    {
        std::string name;
        std::string fullName;
        std::string deployer;
        std::optional<std::string> oracleUpdater;
        std::optional<std::string> feeRecipient;
        std::vector<std::pair<std::string, std::string>> assetToStreamingOiCap;
        std::vector<std::pair<std::string, std::string>> assetToFundingMultiplier;
    };

    struct PerpDexsResponse
    {
        std::vector<PerpDex> dexes;
    };

    struct RestBookLevel
    {
        std::string px;
        std::string sz;
        int n;
    };

    struct L2BookResponse
    {
        std::string coin;
        uint64_t time;
        std::vector<RestBookLevel> bids;
        std::vector<RestBookLevel> asks;
    };

    struct CandleSnapshotResponse
    {
        std::vector<Candle> candles;
    };

    struct AllMidsResponse
    {
        std::vector<AllMidsEntry> mids;
    };

    struct OpenOrdersResponse
    {
        std::vector<OpenOrder> orders;
    };

    struct OrderStatusOrder
    {
        OpenOrder order;
        OrderStatus status;
        uint64_t statusTimestamp;
    };

    struct OrderStatusResponse
    {
        std::string status; // "order" or "unknownOid"
        std::optional<OrderStatusOrder> order;
    };

    struct UserFillsResponse
    {
        std::vector<Fill> fills;
    };

    // --- Rest endpoint types (authenticated) ---

    struct OrderStatusResting
    {
        uint64_t oid;
        std::optional<std::string> cloid;
    };

    struct OrderStatusFilled
    {
        std::string totalSz;
        std::string avgPx;
        uint64_t oid;
        std::optional<std::string> cloid;
    };

    struct OrderStatusResult
    {
        std::optional<OrderStatusResting> resting;
        std::optional<OrderStatusFilled> filled;
        std::optional<std::string> error;
    };

    struct PlaceOrderResponse
    {
        std::string status;
        std::string type;
        std::vector<OrderStatusResult> statuses;
    };

    struct CancelStatusResult
    {
        std::optional<std::string> success;
        std::optional<std::string> error;
    };

    struct CancelOrderResponse
    {
        std::string status;
        std::string type;
        std::vector<CancelStatusResult> statuses;
    };

    struct ModifyOrderResponse
    {
        std::string status;
        std::string type;
        std::vector<OrderStatusResult> statuses;
    };

    struct SimpleResponse
    {
        std::string status;
        std::string type;
        std::optional<std::string> error;
    };
}
