#include <gtest/gtest.h>

#include "hyperliquid/websocket/WebsocketMessageParser.h"
#include "hyperliquid/websocket/WebsocketMessageHandler.h"

using namespace hyperliquid;

namespace
{
    struct CapturingHandler : WebsocketMessageHandler
    {
        std::vector<UserFunding> userFundingUpdates;
        std::vector<LedgerUpdate> ledgerUpdates;
        std::optional<WebData3Update> webData3;
        std::optional<ClearinghouseStateUpdate> clearinghouseState;
        std::optional<OpenOrdersUpdate> openOrders;

        void onUserFundingUpdate(const UserFunding& funding) override
        {
            userFundingUpdates.push_back(funding);
        }

        void onLedgerUpdate(const LedgerUpdate& update) override
        {
            ledgerUpdates.push_back(update);
        }

        void onWebData3(const WebData3Update& update) override
        {
            webData3 = update;
        }

        void onClearinghouseState(const ClearinghouseStateUpdate& update) override
        {
            clearinghouseState = update;
        }

        void onOpenOrdersSnapshot(const OpenOrdersUpdate& update) override
        {
            openOrders = update;
        }
    };
}

TEST(WebsocketParser, UserFundings)
{
    static const std::string kMsg = R"({
        "channel": "userFundings",
        "data": {
            "isSnapshot": true,
            "user": "0x0000000000000000000000000000000000000a",
            "fundings": [
                {"time": 1700000000000, "coin": "ETH", "usdc": "-1.23", "szi": "0.5", "fundingRate": "0.0000125"},
                {"time": 1700000003600, "coin": "BTC", "usdc": "2.5", "szi": "-0.1", "fundingRate": "0.00002"}
            ]
        }
    })";

    WebsocketMessageParser parser;
    CapturingHandler handler;
    parser.crack(kMsg, handler);

    ASSERT_EQ(handler.userFundingUpdates.size(), 2u);

    const auto& first = handler.userFundingUpdates[0];
    EXPECT_TRUE(first.isSnapshot);
    EXPECT_EQ(first.time, 1700000000000u);
    EXPECT_EQ(first.coin, "ETH");
    EXPECT_DOUBLE_EQ(first.usdc, -1.23);
    EXPECT_DOUBLE_EQ(first.szi, 0.5);
    EXPECT_DOUBLE_EQ(first.fundingRate, 0.0000125);

    const auto& second = handler.userFundingUpdates[1];
    EXPECT_TRUE(second.isSnapshot);
    EXPECT_EQ(second.coin, "BTC");
    EXPECT_DOUBLE_EQ(second.usdc, 2.5);
}

TEST(WebsocketParser, UserNonFundingLedgerUpdatesDeposit)
{
    static const std::string kMsg = R"({
        "channel": "userNonFundingLedgerUpdates",
        "data": {
            "isSnapshot": false,
            "user": "0x0000000000000000000000000000000000000a",
            "nonFundingLedgerUpdates": [
                {
                    "time": 1700000000000,
                    "hash": "0xabc123",
                    "delta": {"type": "deposit", "usdc": "100.5"}
                }
            ]
        }
    })";

    WebsocketMessageParser parser;
    CapturingHandler handler;
    parser.crack(kMsg, handler);

    ASSERT_EQ(handler.ledgerUpdates.size(), 1u);
    const auto& update = handler.ledgerUpdates[0];
    EXPECT_FALSE(update.isSnapshot);
    EXPECT_EQ(update.time, 1700000000000u);
    EXPECT_EQ(update.hash, "0xabc123");
    EXPECT_EQ(update.type, LedgerUpdateType::Deposit);
    EXPECT_DOUBLE_EQ(update.usdc, 100.5);
}

TEST(WebsocketParser, UserNonFundingLedgerUpdatesWithdraw)
{
    static const std::string kMsg = R"({
        "channel": "userNonFundingLedgerUpdates",
        "data": {
            "user": "0x0000000000000000000000000000000000000a",
            "nonFundingLedgerUpdates": [
                {
                    "time": 1700000001000,
                    "hash": "0xdef456",
                    "delta": {"type": "withdraw", "usdc": "50", "nonce": 42, "fee": "1.0"}
                }
            ]
        }
    })";

    WebsocketMessageParser parser;
    CapturingHandler handler;
    parser.crack(kMsg, handler);

    ASSERT_EQ(handler.ledgerUpdates.size(), 1u);
    const auto& update = handler.ledgerUpdates[0];
    EXPECT_EQ(update.type, LedgerUpdateType::Withdraw);
    EXPECT_DOUBLE_EQ(update.usdc, 50.0);
    EXPECT_EQ(update.nonce, 42u);
    EXPECT_DOUBLE_EQ(update.fee, 1.0);
}

TEST(WebsocketParser, UserNonFundingLedgerUpdatesLiquidation)
{
    static const std::string kMsg = R"({
        "channel": "userNonFundingLedgerUpdates",
        "data": {
            "user": "0x0000000000000000000000000000000000000a",
            "nonFundingLedgerUpdates": [
                {
                    "time": 1700000002000,
                    "hash": "0xliq789",
                    "delta": {
                        "type": "liquidation",
                        "accountValue": 123.45,
                        "leverageType": "Cross",
                        "liquidatedPositions": [
                            {"coin": "ETH", "szi": "-1.5"},
                            {"coin": "BTC", "szi": "0.02"}
                        ]
                    }
                }
            ]
        }
    })";

    WebsocketMessageParser parser;
    CapturingHandler handler;
    parser.crack(kMsg, handler);

    ASSERT_EQ(handler.ledgerUpdates.size(), 1u);
    const auto& update = handler.ledgerUpdates[0];
    EXPECT_EQ(update.type, LedgerUpdateType::Liquidation);
    EXPECT_DOUBLE_EQ(update.accountValue, 123.45);
    EXPECT_EQ(update.leverageType, LeverageType::Cross);
    ASSERT_EQ(update.liquidatedPositions.size(), 2u);
    EXPECT_EQ(update.liquidatedPositions[0].coin, "ETH");
    EXPECT_DOUBLE_EQ(update.liquidatedPositions[0].szi, -1.5);
    EXPECT_EQ(update.liquidatedPositions[1].coin, "BTC");
}

TEST(WebsocketParser, UserNonFundingLedgerUpdatesSpotTransfer)
{
    static const std::string kMsg = R"({
        "channel": "userNonFundingLedgerUpdates",
        "data": {
            "user": "0x0000000000000000000000000000000000000a",
            "nonFundingLedgerUpdates": [
                {
                    "time": 1700000003000,
                    "hash": "0xspot001",
                    "delta": {
                        "type": "spotTransfer",
                        "token": "PURR",
                        "amount": "10.0",
                        "usdcValue": "5.25",
                        "user": "0xaaaa",
                        "destination": "0xbbbb",
                        "fee": "0.01"
                    }
                }
            ]
        }
    })";

    WebsocketMessageParser parser;
    CapturingHandler handler;
    parser.crack(kMsg, handler);

    ASSERT_EQ(handler.ledgerUpdates.size(), 1u);
    const auto& update = handler.ledgerUpdates[0];
    EXPECT_EQ(update.type, LedgerUpdateType::SpotTransfer);
    EXPECT_EQ(update.token, "PURR");
    EXPECT_DOUBLE_EQ(update.amount, 10.0);
    EXPECT_DOUBLE_EQ(update.usdcValue, 5.25);
    EXPECT_EQ(update.user, "0xaaaa");
    EXPECT_EQ(update.destination, "0xbbbb");
    EXPECT_DOUBLE_EQ(update.fee, 0.01);
}

TEST(WebsocketParser, UserNonFundingLedgerUpdatesSend)
{
    static const std::string kMsg = R"({
        "channel": "userNonFundingLedgerUpdates",
        "data": {
            "user": "0x0000000000000000000000000000000000000a",
            "nonFundingLedgerUpdates": [
                {
                    "time": 1700000004000,
                    "hash": "0xsend001",
                    "delta": {
                        "type": "send",
                        "user": "0xaaaa",
                        "destination": "0xbbbb",
                        "sourceDex": "spot",
                        "destinationDex": "spot",
                        "token": "USDC",
                        "amount": "88.0",
                        "usdcValue": "88.0",
                        "fee": "1.0",
                        "nativeTokenFee": "0.0",
                        "nonce": 1778486945010,
                        "feeToken": "USDC"
                    }
                }
            ]
        }
    })";

    WebsocketMessageParser parser;
    CapturingHandler handler;
    parser.crack(kMsg, handler);

    ASSERT_EQ(handler.ledgerUpdates.size(), 1u);
    const auto& update = handler.ledgerUpdates[0];
    EXPECT_EQ(update.type, LedgerUpdateType::Send);
    EXPECT_EQ(update.user, "0xaaaa");
    EXPECT_EQ(update.destination, "0xbbbb");
    EXPECT_EQ(update.sourceDex, "spot");
    EXPECT_EQ(update.destinationDex, "spot");
    EXPECT_EQ(update.token, "USDC");
    EXPECT_DOUBLE_EQ(update.amount, 88.0);
    EXPECT_DOUBLE_EQ(update.usdcValue, 88.0);
    EXPECT_DOUBLE_EQ(update.fee, 1.0);
    EXPECT_DOUBLE_EQ(update.nativeTokenFee, 0.0);
    EXPECT_EQ(update.nonce, 1778486945010u);
    EXPECT_EQ(update.feeToken, "USDC");
}

TEST(WebsocketParser, WebData3)
{
    static const std::string kMsg = R"({
        "channel": "webData3",
        "data": {
            "userState": {
                "agentAddress": "0xagent",
                "agentValidUntil": 1700099999000,
                "serverTime": 1700000000000,
                "cumLedger": "1000.5",
                "isVault": false,
                "user": "0x0000000000000000000000000000000000000a",
                "optOutOfSpotDusting": true,
                "dexAbstractionEnabled": false
            },
            "perpDexStates": [
                {
                    "totalVaultEquity": "5000.0",
                    "perpsAtOpenInterestCap": ["BTC", "ETH"],
                    "leadingVaults": [
                        {"address": "0xvault1", "name": "Alpha Vault"}
                    ]
                }
            ]
        }
    })";

    WebsocketMessageParser parser;
    CapturingHandler handler;
    parser.crack(kMsg, handler);

    ASSERT_TRUE(handler.webData3.has_value());
    const auto& update = *handler.webData3;
    ASSERT_TRUE(update.userState.agentAddress.has_value());
    EXPECT_EQ(*update.userState.agentAddress, "0xagent");
    ASSERT_TRUE(update.userState.agentValidUntil.has_value());
    EXPECT_EQ(*update.userState.agentValidUntil, 1700099999000u);
    EXPECT_EQ(update.userState.serverTime, 1700000000000u);
    EXPECT_DOUBLE_EQ(update.userState.cumLedger, 1000.5);
    EXPECT_FALSE(update.userState.isVault);
    EXPECT_TRUE(update.userState.optOutOfSpotDusting);
    EXPECT_FALSE(update.userState.dexAbstractionEnabled);

    ASSERT_EQ(update.perpDexStates.size(), 1u);
    EXPECT_DOUBLE_EQ(update.perpDexStates[0].totalVaultEquity, 5000.0);
    ASSERT_EQ(update.perpDexStates[0].perpsAtOpenInterestCap.size(), 2u);
    EXPECT_EQ(update.perpDexStates[0].perpsAtOpenInterestCap[0], "BTC");
    ASSERT_EQ(update.perpDexStates[0].leadingVaults.size(), 1u);
    EXPECT_EQ(update.perpDexStates[0].leadingVaults[0].address, "0xvault1");
    EXPECT_EQ(update.perpDexStates[0].leadingVaults[0].name, "Alpha Vault");

    // Escape hatch: full raw JSON of the data object retained too.
    EXPECT_NE(update.raw.find("agentAddress"), std::string::npos);
}

TEST(WebsocketParser, ClearinghouseState)
{
    static const std::string kMsg = R"({
        "channel": "clearinghouseState",
        "data": {
            "dex": "",
            "user": "0x0000000000000000000000000000000000000a",
            "clearinghouseState": {
                "assetPositions": [
                    {
                        "type": "oneWay",
                        "position": {
                            "coin": "ETH",
                            "szi": "1.5",
                            "entryPx": "2000.0",
                            "positionValue": "3000.0",
                            "unrealizedPnl": "50.0",
                            "returnOnEquity": "0.05",
                            "liquidationPx": "1500.0",
                            "marginUsed": "300.0",
                            "maxLeverage": 20,
                            "leverage": {"type": "cross", "value": 10}
                        }
                    }
                ],
                "marginSummary": {
                    "accountValue": "10000.0",
                    "totalNtlPos": "3000.0",
                    "totalRawUsd": "7000.0",
                    "totalMarginUsed": "300.0"
                },
                "crossMarginSummary": {
                    "accountValue": "10000.0",
                    "totalNtlPos": "3000.0",
                    "totalRawUsd": "7000.0",
                    "totalMarginUsed": "300.0"
                },
                "crossMaintenanceMarginUsed": "150.0",
                "withdrawable": "9700.0"
            }
        }
    })";

    WebsocketMessageParser parser;
    CapturingHandler handler;
    parser.crack(kMsg, handler);

    ASSERT_TRUE(handler.clearinghouseState.has_value());
    const auto& update = *handler.clearinghouseState;
    EXPECT_EQ(update.dex, "");
    EXPECT_EQ(update.user, "0x0000000000000000000000000000000000000a");

    ASSERT_EQ(update.state.assetPositions.size(), 1u);
    const auto& pos = update.state.assetPositions[0];
    EXPECT_EQ(pos.coin, "ETH");
    EXPECT_DOUBLE_EQ(pos.szi, 1.5);
    EXPECT_DOUBLE_EQ(pos.entryPx, 2000.0);
    EXPECT_DOUBLE_EQ(pos.positionValue, 3000.0);
    EXPECT_DOUBLE_EQ(pos.unrealizedPnl, 50.0);
    EXPECT_TRUE(pos.hasLiquidationPx);
    EXPECT_DOUBLE_EQ(pos.liquidationPx, 1500.0);
    EXPECT_DOUBLE_EQ(pos.marginUsed, 300.0);
    EXPECT_EQ(pos.maxLeverage, 20);
    EXPECT_EQ(pos.leverageType, LeverageType::Cross);

    EXPECT_DOUBLE_EQ(update.state.marginSummary.accountValue, 10000.0);
    EXPECT_DOUBLE_EQ(update.state.crossMarginSummary.totalMarginUsed, 300.0);
    EXPECT_DOUBLE_EQ(update.state.crossMaintenanceMarginUsed, 150.0);
    EXPECT_DOUBLE_EQ(update.state.withdrawable, 9700.0);
}

TEST(WebsocketParser, OpenOrders)
{
    static const std::string kMsg = R"({
        "channel": "openOrders",
        "data": {
            "dex": "",
            "user": "0x0000000000000000000000000000000000000a",
            "orders": [
                {
                    "coin": "BTC",
                    "side": "B",
                    "limitPx": "29792.0",
                    "sz": "0.001",
                    "oid": 91490942,
                    "timestamp": 1681247412012,
                    "origSz": "0.001"
                },
                {
                    "coin": "ETH",
                    "side": "A",
                    "limitPx": "2000.0",
                    "sz": "0.5",
                    "oid": 91490943,
                    "timestamp": 1681247412100,
                    "origSz": "1.0",
                    "cloid": "0xdeadbeef"
                }
            ]
        }
    })";

    WebsocketMessageParser parser;
    CapturingHandler handler;
    parser.crack(kMsg, handler);

    ASSERT_TRUE(handler.openOrders.has_value());
    const auto& update = *handler.openOrders;
    EXPECT_EQ(update.user, "0x0000000000000000000000000000000000000a");
    ASSERT_EQ(update.orders.size(), 2u);

    EXPECT_EQ(update.orders[0].coin, "BTC");
    EXPECT_EQ(update.orders[0].side, 'B');
    EXPECT_DOUBLE_EQ(update.orders[0].limitPx, 29792.0);
    EXPECT_EQ(update.orders[0].oid, 91490942u);
    EXPECT_TRUE(update.orders[0].cloid.empty());

    EXPECT_EQ(update.orders[1].coin, "ETH");
    EXPECT_EQ(update.orders[1].side, 'A');
    EXPECT_EQ(update.orders[1].cloid, "0xdeadbeef");
}
