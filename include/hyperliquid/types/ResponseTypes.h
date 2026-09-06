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

    enum class LeverageType { Cross, Isolated, Unknown };

    inline LeverageType stringToLeverageType(std::string_view s)
    {
        if (s == "cross" || s == "Cross") return LeverageType::Cross;
        if (s == "isolated" || s == "Isolated") return LeverageType::Isolated;
        return LeverageType::Unknown;
    }

    inline std::string toString(LeverageType type)
    {
        switch (type)
        {
        case LeverageType::Cross: return "Cross";
        case LeverageType::Isolated: return "Isolated";
        default: return "Unknown";
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
        LeverageType leverageType;
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
        uint64_t id; // only populated for the twapStates channel, where states are keyed as [id, TwapState] pairs
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

    struct TwapStatesUpdate
    {
        std::string dex;
        std::string user;
        std::vector<TwapState> states;
    };

    enum class TwapHistoryStatus { Activated, Terminated, Finished, Error, Unknown };

    inline TwapHistoryStatus stringToTwapHistoryStatus(std::string_view s)
    {
        if (s == "activated") return TwapHistoryStatus::Activated;
        if (s == "terminated") return TwapHistoryStatus::Terminated;
        if (s == "finished") return TwapHistoryStatus::Finished;
        if (s == "error") return TwapHistoryStatus::Error;
        return TwapHistoryStatus::Unknown;
    }

    inline std::string toString(TwapHistoryStatus status)
    {
        switch (status)
        {
        case TwapHistoryStatus::Activated: return "activated";
        case TwapHistoryStatus::Terminated: return "terminated";
        case TwapHistoryStatus::Finished: return "finished";
        case TwapHistoryStatus::Error: return "error";
        default: return "unknown";
        }
    }

    struct TwapHistoryEntry
    {
        TwapState state;
        TwapHistoryStatus status;
        std::string description;
        uint64_t time;
        bool isSnapshot = false;
    };

    struct TwapSliceFill
    {
        Fill fill;
        uint64_t twapId;
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
        Send,
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
        if (s == "send") return LedgerUpdateType::Send;
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
        case LedgerUpdateType::Send: return "send";
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
        double usdcValue;
        uint64_t nonce;
        bool toPerp;
        std::string sourceDex;
        std::string destinationDex;
        double nativeTokenFee;
        std::string feeToken;
        // liquidation-only fields
        double accountValue;
        LeverageType leverageType;
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
        LeverageType leverageType;
        double maxTradeSzLong;
        double maxTradeSzShort;
        double availableToTradeLong;
        double availableToTradeShort;
    };

    struct Notification
    {
        std::string notification;
    };

    struct SpotBalance
    {
        std::string coin;
        int token;
        double hold;
        double total;
        double entryNtl;
    };

    struct SpotStateUpdate
    {
        std::string user;
        std::vector<SpotBalance> balances;
    };

    struct DexClearinghouseState
    {
        std::string dex;
        ClearinghouseState state;
    };

    struct AllDexsClearinghouseStateUpdate
    {
        std::string user;
        std::vector<DexClearinghouseState> states;
    };

    struct DexAssetCtxs
    {
        std::string dex;
        std::vector<PerpAssetCtx> ctxs;
    };

    struct AllDexsAssetCtxsUpdate
    {
        std::vector<DexAssetCtxs> dexs;
    };

    struct FastAssetCtx
    {
        std::string coin;
        bool hasMarkPx;
        double markPx;
        bool hasMidPx;
        double midPx;
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
        std::optional<int> token;
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

    // Parses the pipe-delimited "class:X|underlying:Y|expiry:.../targetPrice:.../period:..." format
    // shared by outcomeMeta, settledOutcome, and outcomeMetaUpdates outcome descriptions.
    inline OutcomeDescription parseOutcomeDescription(const std::string& desc)
    {
        OutcomeDescription result;
        size_t pos = 0;
        while (pos < desc.size())
        {
            size_t sep = desc.find('|', pos);
            std::string_view segment(desc.data() + pos, (sep == std::string::npos ? desc.size() : sep) - pos);
            size_t colon = segment.find(':');
            if (colon != std::string_view::npos)
            {
                std::string_view key = segment.substr(0, colon);
                std::string_view val = segment.substr(colon + 1);
                if (key == "class") result.outcomeClass = std::string(val);
                else if (key == "underlying") result.underlying = std::string(val);
                else if (key == "expiry") result.expiry = parseOutcomeExpiry(std::string(val));
                else if (key == "targetPrice") result.targetPrice = std::string(val);
                else if (key == "period") result.period = std::string(val);
            }
            if (sep == std::string::npos) break;
            pos = sep + 1;
        }
        return result;
    }

    struct Outcome
    {
        int outcome;
        std::string name;
        std::string descriptionRaw;
        OutcomeDescription description;
        std::vector<OutcomeSideSpec> sideSpecs;
        std::string quoteToken;
        std::optional<std::string> deployer;
    };

    struct OutcomeMetaResponse
    {
        std::vector<Outcome> outcomes;
    };

    struct QuestionSpec
    {
        int question;
        std::string name;
        std::string description;
        int fallbackOutcome;
        std::vector<int> namedOutcomes;
        std::vector<int> settledNamedOutcomes;
    };

    // Flattened representation of settledOutcome's optional `question` field, whose
    // `question.question` key is `active` or `settled` depending on question state.
    struct SettledOutcomeQuestion
    {
        bool isSettled = false;
        int questionId = 0;
        std::string name;
        std::string description;
    };

    struct SettledOutcomeResponse
    {
        bool isSettled = false;
        Outcome spec;
        double settleFraction = 0.0;
        std::string details;
        std::optional<SettledOutcomeQuestion> question;
    };

    enum class OutcomeMetaUpdateType
    {
        OutcomeCreated,
        OutcomeSettled,
        QuestionUpdated,
        QuestionSettled,
        Unknown
    };

    inline std::string toString(OutcomeMetaUpdateType type)
    {
        switch (type)
        {
        case OutcomeMetaUpdateType::OutcomeCreated: return "outcomeCreated";
        case OutcomeMetaUpdateType::OutcomeSettled: return "outcomeSettled";
        case OutcomeMetaUpdateType::QuestionUpdated: return "questionUpdated";
        case OutcomeMetaUpdateType::QuestionSettled: return "questionSettled";
        default: return "unknown";
        }
    }

    inline OutcomeMetaUpdateType stringToOutcomeMetaUpdateType(std::string_view s)
    {
        if (s == "outcomeCreated") return OutcomeMetaUpdateType::OutcomeCreated;
        if (s == "outcomeSettled") return OutcomeMetaUpdateType::OutcomeSettled;
        if (s == "questionUpdated") return OutcomeMetaUpdateType::QuestionUpdated;
        if (s == "questionSettled") return OutcomeMetaUpdateType::QuestionSettled;
        return OutcomeMetaUpdateType::Unknown;
    }

    // Flattened representation of the WsOutcomeMetaUpdate discriminated union
    // (outcomeCreated/outcomeSettled/questionUpdated/questionSettled). `type`
    // indicates which variant this was; fields not applicable are left default.
    struct OutcomeMetaUpdate
    {
        OutcomeMetaUpdateType type;
        Outcome outcome;
        int settledOutcome = 0;
        QuestionSpec question;
        int settledQuestion = 0;
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

    struct PerpsAtOpenInterestCapResponse
    {
        std::vector<std::string> coins;
    };

    struct PredictedFundingVenue
    {
        std::string venue;
        std::optional<double> fundingRate;
        std::optional<uint64_t> nextFundingTime;
    };

    struct PredictedFundingEntry
    {
        std::string coin;
        std::vector<PredictedFundingVenue> venues;
    };

    struct PredictedFundingsResponse
    {
        std::vector<PredictedFundingEntry> fundings;
    };

    // "category" is dex-defined and open-ended (builder-deployed dexs can invent new
    // categories), so it is kept as a string rather than a fixed enum.
    struct PerpAnnotationResponse
    {
        std::string category;
        std::string description;
    };

    struct PerpCategoryEntry
    {
        std::string coin;
        std::string category;
    };

    struct PerpCategoriesResponse
    {
        std::vector<PerpCategoryEntry> categories;
    };

    struct PerpConciseAnnotationEntry
    {
        std::string coin;
        std::string category;
        std::vector<std::string> keywords;
    };

    struct PerpConciseAnnotationsResponse
    {
        std::vector<PerpConciseAnnotationEntry> annotations;
    };

    struct MarginTier
    {
        double lowerBound;
        int maxLeverage;
    };

    struct MarginTableEntry
    {
        int id;
        std::string description;
        std::vector<MarginTier> marginTiers;
    };

    struct PerpDexMeta
    {
        std::vector<AssetMeta> universe;
        std::vector<MarginTableEntry> marginTables;
        int collateralToken;
    };

    struct AllPerpMetasResponse
    {
        std::vector<PerpDexMeta> dexMetas;
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

    struct UserRateLimitResponse
    {
        double cumVlm;
        int64_t nRequestsUsed;
        int64_t nRequestsCap;
        int64_t nRequestsSurplus;
    };

    struct MetaAndAssetCtxsResponse
    {
        MetaResponse meta;
        std::vector<PerpAssetCtx> assetCtxs;
    };

    struct SpotUniversePair
    {
        std::string name;
        std::vector<int> tokens;
        int index;
        bool isCanonical;
    };

    struct SpotMetaAndAssetCtxsMeta
    {
        std::vector<SpotAssetMeta> tokens;
        std::vector<SpotUniversePair> universe;
    };

    struct SpotMetaAndAssetCtxsResponse
    {
        SpotMetaAndAssetCtxsMeta meta;
        std::vector<SpotAssetCtx> assetCtxs;
    };

    struct SpotClearinghouseStateResponse
    {
        std::vector<SpotBalance> balances;
    };

    enum class FrontendOrderType { Market, Limit, StopMarket, StopLimit, TakeProfitMarket, TakeProfitLimit, Unknown };

    inline FrontendOrderType stringToFrontendOrderType(std::string_view s)
    {
        if (s == "Market") return FrontendOrderType::Market;
        if (s == "Limit") return FrontendOrderType::Limit;
        if (s == "Stop Market") return FrontendOrderType::StopMarket;
        if (s == "Stop Limit") return FrontendOrderType::StopLimit;
        if (s == "Take Profit Market") return FrontendOrderType::TakeProfitMarket;
        if (s == "Take Profit Limit") return FrontendOrderType::TakeProfitLimit;
        return FrontendOrderType::Unknown;
    }

    inline std::string toString(FrontendOrderType type)
    {
        switch (type)
        {
        case FrontendOrderType::Market: return "Market";
        case FrontendOrderType::Limit: return "Limit";
        case FrontendOrderType::StopMarket: return "Stop Market";
        case FrontendOrderType::StopLimit: return "Stop Limit";
        case FrontendOrderType::TakeProfitMarket: return "Take Profit Market";
        case FrontendOrderType::TakeProfitLimit: return "Take Profit Limit";
        default: return "Unknown";
        }
    }

    enum class OrderTif { Gtc, Ioc, Alo, FrontendMarket, LiquidationMarket, Unknown };

    inline OrderTif stringToOrderTif(std::string_view s)
    {
        if (s == "Gtc") return OrderTif::Gtc;
        if (s == "Ioc") return OrderTif::Ioc;
        if (s == "Alo") return OrderTif::Alo;
        if (s == "FrontendMarket") return OrderTif::FrontendMarket;
        if (s == "Liquidation Market") return OrderTif::LiquidationMarket;
        return OrderTif::Unknown;
    }

    inline std::string toString(OrderTif tif)
    {
        switch (tif)
        {
        case OrderTif::Gtc: return "Gtc";
        case OrderTif::Ioc: return "Ioc";
        case OrderTif::Alo: return "Alo";
        case OrderTif::FrontendMarket: return "FrontendMarket";
        case OrderTif::LiquidationMarket: return "Liquidation Market";
        default: return "Unknown";
        }
    }

    struct FrontendOrder
    {
        std::string coin;
        char side;
        double limitPx;
        double sz;
        uint64_t oid;
        uint64_t timestamp;
        double origSz;
        std::string cloid;
        bool isPositionTpsl;
        bool isTrigger;
        double triggerPx;
        std::string triggerCondition;
        bool reduceOnly;
        FrontendOrderType orderType;
        std::optional<OrderTif> tif;
    };

    struct FrontendOpenOrdersResponse
    {
        std::vector<FrontendOrder> orders;
    };

    struct HistoricalOrder
    {
        FrontendOrder order;
        OrderStatus status;
        uint64_t statusTimestamp;
    };

    struct HistoricalOrdersResponse
    {
        std::vector<HistoricalOrder> orders;
    };

    struct UserTwapSliceFillsResponse
    {
        std::vector<TwapSliceFill> fills;
    };

    struct SubAccount
    {
        std::string name;
        std::string subAccountUser;
        std::string master;
        ClearinghouseState clearinghouseState;
        SpotClearinghouseStateResponse spotState;
    };

    struct SubAccountsResponse
    {
        std::vector<SubAccount> subAccounts;
    };

    struct DailyUserVolume
    {
        std::string date;
        double userCross;
        double userAdd;
        double exchange;
    };

    struct FeeTierVip
    {
        double ntlCutoff;
        double cross;
        double add;
        double spotCross;
        double spotAdd;
    };

    struct FeeTierMm
    {
        double makerFractionCutoff;
        double add;
    };

    struct StakingDiscountTier
    {
        double bpsOfMaxSupply;
        double discount;
    };

    struct FeeSchedule
    {
        double cross;
        double add;
        double spotCross;
        double spotAdd;
        std::vector<FeeTierVip> vipTiers;
        std::vector<FeeTierMm> mmTiers;
        double referralDiscount;
        std::vector<StakingDiscountTier> stakingDiscountTiers;
    };

    struct StakingLink
    {
        std::string type;
        std::string stakingUser;
    };

    struct ActiveStakingDiscount
    {
        double bpsOfMaxSupply;
        double discount;
    };

    struct UserFeesResponse
    {
        std::vector<DailyUserVolume> dailyUserVlm;
        FeeSchedule feeSchedule;
        double userCrossRate;
        double userAddRate;
        double userSpotCrossRate;
        double userSpotAddRate;
        double activeReferralDiscount;
        double feeTrialEscrow;
        std::optional<uint64_t> nextTrialAvailableTimestamp;
        std::optional<StakingLink> stakingLink;
        std::optional<ActiveStakingDiscount> activeStakingDiscount;
    };

    struct MaxBuilderFeeResponse
    {
        int maxFeeRateTenthsBps;
    };

    struct ApprovedBuildersResponse
    {
        std::vector<std::string> builders;
    };

    enum class PortfolioPeriodType { Day, Week, Month, AllTime, PerpDay, PerpWeek, PerpMonth, PerpAllTime, Unknown };

    inline PortfolioPeriodType stringToPortfolioPeriodType(std::string_view s)
    {
        if (s == "day") return PortfolioPeriodType::Day;
        if (s == "week") return PortfolioPeriodType::Week;
        if (s == "month") return PortfolioPeriodType::Month;
        if (s == "allTime") return PortfolioPeriodType::AllTime;
        if (s == "perpDay") return PortfolioPeriodType::PerpDay;
        if (s == "perpWeek") return PortfolioPeriodType::PerpWeek;
        if (s == "perpMonth") return PortfolioPeriodType::PerpMonth;
        if (s == "perpAllTime") return PortfolioPeriodType::PerpAllTime;
        return PortfolioPeriodType::Unknown;
    }

    inline std::string toString(PortfolioPeriodType type)
    {
        switch (type)
        {
        case PortfolioPeriodType::Day: return "day";
        case PortfolioPeriodType::Week: return "week";
        case PortfolioPeriodType::Month: return "month";
        case PortfolioPeriodType::AllTime: return "allTime";
        case PortfolioPeriodType::PerpDay: return "perpDay";
        case PortfolioPeriodType::PerpWeek: return "perpWeek";
        case PortfolioPeriodType::PerpMonth: return "perpMonth";
        case PortfolioPeriodType::PerpAllTime: return "perpAllTime";
        default: return "unknown";
        }
    }

    struct PortfolioPeriodMetrics
    {
        PortfolioPeriodType period;
        std::vector<std::pair<uint64_t, double>> accountValueHistory;
        std::vector<std::pair<uint64_t, double>> pnlHistory;
        double vlm;
    };

    struct PortfolioResponse
    {
        std::vector<PortfolioPeriodMetrics> periods;
    };

    struct VaultFollower
    {
        std::string user;
        double vaultEquity;
        double pnl;
        double allTimePnl;
        int daysFollowing;
        uint64_t vaultEntryTime;
        uint64_t lockupUntil;
    };

    // "relationship.data" varies by relationship.type ("parent" is the only documented
    // variant); only childAddresses is modeled, the rest is left at defaults.
    struct VaultRelationship
    {
        std::string type;
        std::vector<std::string> childAddresses;
    };

    struct VaultDetailsResponse
    {
        std::string name;
        std::string vaultAddress;
        std::string leader;
        std::string description;
        std::vector<PortfolioPeriodMetrics> portfolio;
        double apr;
        std::optional<std::string> followerStateRaw; // raw JSON of followerState; undocumented shape, null when absent
        double leaderFraction;
        double leaderCommission;
        std::vector<VaultFollower> followers;
        double maxDistributable;
        double maxWithdrawable;
        bool isClosed;
        VaultRelationship relationship;
        bool allowDeposits;
        bool alwaysCloseOnWithdraw;
    };

    struct UserVaultEquity
    {
        std::string vaultAddress;
        double equity;
    };

    struct UserVaultEquitiesResponse
    {
        std::vector<UserVaultEquity> equities;
    };

    struct ReferredBy
    {
        std::string referrer;
        std::string code;
    };

    struct ReferralState
    {
        double cumVlm;
        double cumRewardedFeesSinceReferred;
        double cumFeesRewardedToReferrer;
        uint64_t timeJoined;
        std::string user;
    };

    // `data`'s shape depends on `stage` - e.g. "needToTrade" carries `required`, other stages
    // (not yet observed against a live account) are assumed to carry `code`/`referralStates`
    // per the wire shape used elsewhere in this response. All fields are optional since the
    // full set of stages and their shapes isn't documented anywhere.
    struct ReferrerState
    {
        std::string stage;
        std::optional<double> required;
        std::optional<std::string> code;
        std::vector<ReferralState> referralStates;
    };

    struct TokenRewardState
    {
        double cumVlm;
        double unclaimedRewards;
        double claimedRewards;
        double builderRewards;
    };

    struct ReferralResponse
    {
        std::optional<ReferredBy> referredBy;
        double cumVlm;
        double unclaimedRewards;
        double claimedRewards;
        double builderRewards;
        // Wire shape is an array containing one [tokenIndex, state] tuple, e.g. [[0, {...}]].
        std::optional<std::pair<int, TokenRewardState>> tokenToState;
        std::optional<ReferrerState> referrerState;
    };

    enum class UserRoleType { User, Agent, Vault, SubAccount, Missing, Unknown };

    inline UserRoleType stringToUserRoleType(std::string_view s)
    {
        if (s == "user") return UserRoleType::User;
        if (s == "agent") return UserRoleType::Agent;
        if (s == "vault") return UserRoleType::Vault;
        if (s == "subAccount") return UserRoleType::SubAccount;
        if (s == "missing") return UserRoleType::Missing;
        return UserRoleType::Unknown;
    }

    inline std::string toString(UserRoleType type)
    {
        switch (type)
        {
        case UserRoleType::User: return "user";
        case UserRoleType::Agent: return "agent";
        case UserRoleType::Vault: return "vault";
        case UserRoleType::SubAccount: return "subAccount";
        case UserRoleType::Missing: return "missing";
        default: return "unknown";
        }
    }

    struct UserRoleResponse
    {
        UserRoleType role;
        std::optional<std::string> agentUser;
        std::optional<std::string> subAccountMaster;
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

    struct TwapOrderResponse
    {
        std::string status;
        std::string type;
        std::optional<uint64_t> twapId;
        std::optional<std::string> error;
    };

    struct TwapCancelResponse
    {
        std::string status;
        std::string type;
        std::optional<std::string> success;
        std::optional<std::string> error;
    };

    // --- Staking / delegation types ---

    struct Delegation
    {
        std::string validator;
        double amount;
        uint64_t lockedUntilTimestamp;
    };

    struct DelegationsResponse
    {
        std::vector<Delegation> delegations;
    };

    struct DelegatorSummaryResponse
    {
        double delegated;
        double undelegated;
        double totalPendingWithdrawal;
        int nPendingWithdrawals;
    };

    enum class DelegatorHistoryDeltaType { Delegate, CDeposit, Withdrawal, Unknown };

    inline DelegatorHistoryDeltaType stringToDelegatorHistoryDeltaType(std::string_view s)
    {
        if (s == "delegate") return DelegatorHistoryDeltaType::Delegate;
        if (s == "cDeposit") return DelegatorHistoryDeltaType::CDeposit;
        if (s == "withdrawal") return DelegatorHistoryDeltaType::Withdrawal;
        return DelegatorHistoryDeltaType::Unknown;
    }

    inline std::string toString(DelegatorHistoryDeltaType type)
    {
        switch (type)
        {
        case DelegatorHistoryDeltaType::Delegate: return "delegate";
        case DelegatorHistoryDeltaType::CDeposit: return "cDeposit";
        case DelegatorHistoryDeltaType::Withdrawal: return "withdrawal";
        default: return "unknown";
        }
    }

    enum class WithdrawalPhase { Initiated, Finalized, Unknown };

    inline WithdrawalPhase stringToWithdrawalPhase(std::string_view s)
    {
        if (s == "initiated") return WithdrawalPhase::Initiated;
        if (s == "finalized") return WithdrawalPhase::Finalized;
        return WithdrawalPhase::Unknown;
    }

    inline std::string toString(WithdrawalPhase phase)
    {
        switch (phase)
        {
        case WithdrawalPhase::Initiated: return "initiated";
        case WithdrawalPhase::Finalized: return "finalized";
        default: return "unknown";
        }
    }

    // Flattened representation of the {delegate|cDeposit|withdrawal}-keyed delta union;
    // `type` indicates which variant this was, fields not applicable to it are left default.
    struct DelegatorHistoryDelta
    {
        DelegatorHistoryDeltaType type;
        std::string validator;
        double amount;
        bool isUndelegate;
        WithdrawalPhase phase;
    };

    struct DelegatorHistoryEntry
    {
        uint64_t time;
        std::string hash;
        DelegatorHistoryDelta delta;
    };

    struct DelegatorHistoryResponse
    {
        std::vector<DelegatorHistoryEntry> history;
    };

    enum class DelegatorRewardSource { Delegation, Commission, Unknown };

    inline DelegatorRewardSource stringToDelegatorRewardSource(std::string_view s)
    {
        if (s == "delegation") return DelegatorRewardSource::Delegation;
        if (s == "commission") return DelegatorRewardSource::Commission;
        return DelegatorRewardSource::Unknown;
    }

    inline std::string toString(DelegatorRewardSource source)
    {
        switch (source)
        {
        case DelegatorRewardSource::Delegation: return "delegation";
        case DelegatorRewardSource::Commission: return "commission";
        default: return "unknown";
        }
    }

    struct DelegatorReward
    {
        uint64_t time;
        DelegatorRewardSource source;
        double totalAmount;
    };

    struct DelegatorRewardsResponse
    {
        std::vector<DelegatorReward> rewards;
    };

    // --- Borrow/lend ---

    struct BorrowLendReserveState
    {
        double borrowYearlyRate;
        double supplyYearlyRate;
        double balance;
        double utilization;
        double oraclePx;
        double ltv;
        double totalSupplied;
        double totalBorrowed;
    };

    struct BorrowLendReserveEntry
    {
        int token;
        BorrowLendReserveState state;
    };

    struct AllBorrowLendReserveStatesResponse
    {
        std::vector<BorrowLendReserveEntry> reserves;
    };

    struct BorrowLendPositionSide
    {
        double basis;
        double value;
    };

    struct BorrowLendUserPosition
    {
        int token;
        BorrowLendPositionSide borrow;
        BorrowLendPositionSide supply;
    };

    struct BorrowLendUserStateResponse
    {
        std::vector<BorrowLendUserPosition> tokenToState;
        std::string health;
        std::optional<double> healthFactor;
    };

    // --- HIP-3 deployer (perp dex abstraction) ---

    struct PerpDexLimitsCoinCap
    {
        std::string coin;
        double oiCap = 0.0;
    };

    // The API returns a bare JSON null for the main dex (empty string) or an unknown dex name;
    // `exists` is false in that case and the other fields are left default.
    struct PerpDexLimitsResponse
    {
        bool exists = false;
        double totalOiCap = 0.0;
        double oiSzCapPerPerp = 0.0;
        double maxTransferNtl = 0.0;
        std::vector<PerpDexLimitsCoinCap> coinToOiCap;
    };

    // The API returns a bare JSON null for an unknown dex name; `exists` is false in that case.
    struct PerpDexStatusResponse
    {
        bool exists = false;
        double totalNetDeposit = 0.0;
    };

    struct PerpDeployAuctionStatusResponse
    {
        uint64_t startTimeSeconds = 0;
        uint64_t durationSeconds = 0;
        double startGas = 0.0;
        std::optional<double> currentGas;
        std::optional<double> endGas;
    };
}
