#pragma once

#include <cstdint>
#include <string>

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


    // --- Market Data types (no auth required) ---

    enum class Side { Bid, Ask };

    struct PriceLevel
    {
        Side side;
        std::string px;
        std::string sz;
        int n;
    };

    struct L2BookUpdate
    {
        std::string coin;
        uint64_t time;
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
        std::string px;
        std::string sz;
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

    // --- User / Trading types (require user address) ---

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
        std::string liquidationMethod;
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
        std::string status;
        uint64_t statusTimestamp;
    };

    struct UserFunding
    {
        uint64_t time;
        std::string coin;
        double usdc;
        double szi;
        double fundingRate;
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
    };

    struct ClearinghouseState
    {
        MarginSummary marginSummary;
        MarginSummary crossMarginSummary;
        double crossMaintenanceMarginUsed;
        double withdrawable;
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
        RewardsClaim
    };

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
        uint64_t nonce;
        bool toPerp;
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
}
