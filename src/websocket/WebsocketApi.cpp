#include "hyperliquid/websocket/WebsocketApi.h"

#include <simdjson.h>

#include "hyperliquid/websocket/WebsocketApiListener.h"
#include "WebsocketRunner.h"
#include <nlohmann/json.hpp>
#include <thread>

#include "config/Logger.h"
#include "hyperliquid/rest/RestApi.h"
#include "messages/ExchangeRequestBuilder.h"
#include "messages/InfoRequestBuilder.h"
#include "signing/Signing.h"

namespace hyperliquid
{
    struct PostRequestInfo
    {
        RestEndpointType type;
        std::optional<uint64_t> correlationId;
    };

    struct WebsocketApi::Impl : internal::WSListener
    {
        internal::WebsocketRunner ws;
        WebsocketApiListener& listener;
        bool stopping = false;
        std::thread thread;
        ExchangeRequestBuilder exchangeRequestBuilder;
        const ApiConfig& config;
        std::atomic<int> postRequestCounter;
        std::unordered_map<uint64_t, PostRequestInfo> postRequestInfo;
        simdjson::ondemand::parser sjParser;
        simdjson::padded_string sjPadded;

        Impl(ApiConfig& config, WebsocketApiListener& listener)
            : ws(config, *this), listener(listener), config(config), postRequestCounter(0)
        {
            config.skipBuildingSymbolMap = true;
            RestApi restApi(config);
            exchangeRequestBuilder.initializeMapping(config, &restApi);
        }

        ~Impl()
        {
            ws.stop();
            if (thread.joinable()) thread.join();
        }

        void onWsMessage(std::string& message) override
        {
            if (stopping) return;
            getLogger()->debug("ws recv: {}", message);

            try
            {
                sjPadded = simdjson::padded_string(message.data(), message.size());
                auto doc = sjParser.iterate(sjPadded);
                std::string_view channel = doc["channel"].get_string().value();

                if (channel == "pong")
                {
                    ws.onPongReceived();
                    return;
                }

                if (channel == "post")
                {
                    auto data = doc["data"].get_object().value();
                    uint64_t id = data["id"].get_uint64().value();
                    auto payload = simdjson::to_json_string(data["response"]["payload"]);
                    auto it = postRequestInfo.find(id);
                    if (it != postRequestInfo.end())
                    {
                        auto info = it->second;
                        postRequestInfo.erase(it);
                        std::string payloadStr(payload.value());
                        listener.onPostResponse(payloadStr, info.type, info.correlationId);
                    }
                    else
                    {
                        getLogger()->error("post response with unknown id: {}", id);
                        listener.onMessage(message);
                    }
                    return;
                }
            }
            catch (const simdjson::simdjson_error& e)
            {
                getLogger()->error("failed to parse ws message: {}", e.what());
            }

            listener.onMessage(message);
        }

        void onWsConnected() override
        {
            listener.onConnected();
        }

        void onWsDisconnected(bool hasError, const std::string& errMsg) override
        {
            listener.onDisconnected(hasError, errMsg);
        }

        void signAndSend(RestEndpointType type, nlohmann::ordered_json body,
                         const std::optional<std::string>& vaultAddress = std::nullopt,
                         const std::optional<uint64_t>& expiresAfter = std::nullopt,
                         std::optional<uint64_t> correlationId = std::nullopt)
        {
            auto effectiveVault = vaultAddress ? vaultAddress : config.vaultAddress;
            auto payload = Signing::prepareBody(config, type, std::move(body), effectiveVault, expiresAfter);
            auto payloadType = isAuthenticated(type) ? "action" : "info";
            int postRequestId = postRequestCounter.fetch_add(1);
            postRequestInfo[postRequestId] = {type, correlationId};
            nlohmann::ordered_json wrapped = {
                {"method", "post"},
                {"id", postRequestId},
                {"request", {
                    {"type", payloadType},
                    {"payload", payload}
                }}
            };
            ws.send(wrapped.dump());
        }
    };

    WebsocketApi::WebsocketApi(ApiConfig& config, WebsocketApiListener& listener) : impl_(
        std::make_unique<Impl>(config, listener))
    {
    }

    WebsocketApi::~WebsocketApi() = default;

    static bool isUserSubscription(SubscriptionType type)
    {
        switch (type)
        {
        case SubscriptionType::OrderUpdates:
        case SubscriptionType::UserEvents:
        case SubscriptionType::UserFills:
        case SubscriptionType::UserFundings:
        case SubscriptionType::UserNonFundingLedgerUpdates:
        case SubscriptionType::Notification:
        case SubscriptionType::WebData3:
        case SubscriptionType::TwapStates:
        case SubscriptionType::ClearingHouseState:
        case SubscriptionType::OpenOrders:
        case SubscriptionType::ActiveAssetData:
        case SubscriptionType::UserTwapSliceFills:
        case SubscriptionType::UserTwapHistory:
        case SubscriptionType::SpotState:
        case SubscriptionType::AllDexsClearinghouseState:
            return true;
        default:
            return false;
        }
    }

    void WebsocketApi::subscribe(const SubscriptionType type, const std::map<std::string, std::string>& filters)
    {
        nlohmann::json subscribeMsg = {
            {"method", "subscribe"},
            {
                "subscription", {
                    {"type", toString(type)},
                }
            }
        };
        if (isUserSubscription(type) && impl_->config.wallet.has_value()
            && filters.find("user") == filters.end())
        {
            subscribeMsg["subscription"]["user"] = impl_->config.wallet->accountAddress;
        }
        for (const auto& [key, value] : filters)
        {
            subscribeMsg["subscription"][key] = value;
        }
        impl_->ws.send(subscribeMsg.dump());
    }

    void WebsocketApi::unsubscribe(const SubscriptionType type, const std::map<std::string, std::string>& filters)
    {
        nlohmann::json unsubscribeMsg = {
            {"method", "unsubscribe"},
            {
                "subscription", {
                    {"type", toString(type)},
                }
            }
        };
        if (isUserSubscription(type) && impl_->config.wallet.has_value()
            && filters.find("user") == filters.end())
        {
            unsubscribeMsg["subscription"]["user"] = impl_->config.wallet->accountAddress;
        }
        for (const auto& [key, value] : filters)
        {
            unsubscribeMsg["subscription"][key] = value;
        }
        impl_->ws.send(unsubscribeMsg.dump());
    }

    void WebsocketApi::start()
    {
        impl_->thread = std::thread([this]()
        {
            impl_->ws.start();
        });
    }

    void WebsocketApi::stop()
    {
        impl_->stopping = true;
        impl_->ws.stop();
        if (impl_->thread.joinable()) impl_->thread.join();
    }

    void WebsocketApi::spotMeta(std::optional<uint64_t> correlationId)
    {
        return impl_->signAndSend(RestEndpointType::SpotMeta, InfoRequestBuilder::spotMeta(),
                                  std::nullopt, std::nullopt, correlationId);
    }

    void WebsocketApi::meta(const std::optional<std::string>& dex,
                            std::optional<uint64_t> correlationId)
    {
        return impl_->signAndSend(RestEndpointType::Meta, InfoRequestBuilder::meta(dex),
                                  std::nullopt, std::nullopt, correlationId);
    }

    void WebsocketApi::outcomeMeta(std::optional<uint64_t> correlationId)
    {
        return impl_->signAndSend(RestEndpointType::OutcomeMeta, InfoRequestBuilder::outcomeMeta(),
                                  std::nullopt, std::nullopt, correlationId);
    }

    void WebsocketApi::perpDexs(std::optional<uint64_t> correlationId)
    {
        return impl_->signAndSend(RestEndpointType::PerpDexs, InfoRequestBuilder::perpDexs(),
                                  std::nullopt, std::nullopt, correlationId);
    }

    void WebsocketApi::settledOutcome(int outcome, std::optional<uint64_t> correlationId)
    {
        return impl_->signAndSend(RestEndpointType::SettledOutcome, InfoRequestBuilder::settledOutcome(outcome),
                                  std::nullopt, std::nullopt, correlationId);
    }

    void WebsocketApi::perpsAtOpenInterestCap(const std::optional<std::string>& dex,
                                              std::optional<uint64_t> correlationId)
    {
        return impl_->signAndSend(RestEndpointType::PerpsAtOpenInterestCap,
                                  InfoRequestBuilder::perpsAtOpenInterestCap(dex),
                                  std::nullopt, std::nullopt, correlationId);
    }

    void WebsocketApi::predictedFundings(std::optional<uint64_t> correlationId)
    {
        return impl_->signAndSend(RestEndpointType::PredictedFundings, InfoRequestBuilder::predictedFundings(),
                                  std::nullopt, std::nullopt, correlationId);
    }

    void WebsocketApi::perpAnnotation(const std::string& coin, std::optional<uint64_t> correlationId)
    {
        return impl_->signAndSend(RestEndpointType::PerpAnnotation, InfoRequestBuilder::perpAnnotation(coin),
                                  std::nullopt, std::nullopt, correlationId);
    }

    void WebsocketApi::perpCategories(std::optional<uint64_t> correlationId)
    {
        return impl_->signAndSend(RestEndpointType::PerpCategories, InfoRequestBuilder::perpCategories(),
                                  std::nullopt, std::nullopt, correlationId);
    }

    void WebsocketApi::perpConciseAnnotations(std::optional<uint64_t> correlationId)
    {
        return impl_->signAndSend(RestEndpointType::PerpConciseAnnotations,
                                  InfoRequestBuilder::perpConciseAnnotations(),
                                  std::nullopt, std::nullopt, correlationId);
    }

    void WebsocketApi::allPerpMetas(std::optional<uint64_t> correlationId)
    {
        return impl_->signAndSend(RestEndpointType::AllPerpMetas, InfoRequestBuilder::allPerpMetas(),
                                  std::nullopt, std::nullopt, correlationId);
    }

    void WebsocketApi::perpDexLimits(const std::string& dex, std::optional<uint64_t> correlationId)
    {
        return impl_->signAndSend(RestEndpointType::PerpDexLimits, InfoRequestBuilder::perpDexLimits(dex),
                                  std::nullopt, std::nullopt, correlationId);
    }

    void WebsocketApi::perpDexStatus(const std::string& dex, std::optional<uint64_t> correlationId)
    {
        return impl_->signAndSend(RestEndpointType::PerpDexStatus, InfoRequestBuilder::perpDexStatus(dex),
                                  std::nullopt, std::nullopt, correlationId);
    }

    void WebsocketApi::perpDeployAuctionStatus(std::optional<uint64_t> correlationId)
    {
        return impl_->signAndSend(RestEndpointType::PerpDeployAuctionStatus,
                                  InfoRequestBuilder::perpDeployAuctionStatus(),
                                  std::nullopt, std::nullopt, correlationId);
    }

    void WebsocketApi::l2Book(const std::string& coin,
                              const std::optional<int>& nSigFigs,
                              const std::optional<int>& mantissa,
                              std::optional<uint64_t> correlationId)
    {
        return impl_->signAndSend(RestEndpointType::L2Book, InfoRequestBuilder::l2Book(coin, nSigFigs, mantissa),
                                  std::nullopt, std::nullopt, correlationId);
    }

    void WebsocketApi::candleSnapshot(const std::string& coin,
                                      const std::string& interval,
                                      uint64_t startTime,
                                      uint64_t endTime,
                                      std::optional<uint64_t> correlationId)
    {
        return impl_->signAndSend(RestEndpointType::CandleSnapshot,
                                  InfoRequestBuilder::candleSnapshot(coin, interval, startTime, endTime),
                                  std::nullopt, std::nullopt, correlationId);
    }

    void WebsocketApi::allMids(const std::optional<std::string>& dex, std::optional<uint64_t> correlationId)
    {
        return impl_->signAndSend(RestEndpointType::AllMids, InfoRequestBuilder::allMids(dex),
                                  std::nullopt, std::nullopt, correlationId);
    }

    void WebsocketApi::openOrders(const std::string& user, const std::optional<std::string>& dex,
                                  std::optional<uint64_t> correlationId)
    {
        return impl_->signAndSend(RestEndpointType::OpenOrders, InfoRequestBuilder::openOrders(user, dex),
                                  std::nullopt, std::nullopt, correlationId);
    }

    void WebsocketApi::orderStatus(const std::string& user, const OrderId& oid,
                                   std::optional<uint64_t> correlationId)
    {
        return impl_->signAndSend(RestEndpointType::OrderStatus, InfoRequestBuilder::orderStatus(user, oid),
                                  std::nullopt, std::nullopt, correlationId);
    }

    void WebsocketApi::userFills(const std::string& user,
                                 const std::optional<bool>& aggregateByTime,
                                 const std::optional<std::string>& dex,
                                 std::optional<uint64_t> correlationId)
    {
        return impl_->signAndSend(RestEndpointType::UserFills,
                                  InfoRequestBuilder::userFills(user, aggregateByTime, dex),
                                  std::nullopt, std::nullopt, correlationId);
    }

    void WebsocketApi::userFillsByTime(const std::string& user,
                                       uint64_t startTime,
                                       const std::optional<uint64_t>& endTime,
                                       const std::optional<bool>& aggregateByTime,
                                       const std::optional<std::string>& dex,
                                       std::optional<uint64_t> correlationId)
    {
        return impl_->signAndSend(RestEndpointType::UserFillsByTime,
                                  InfoRequestBuilder::userFillsByTime(user, startTime, endTime, aggregateByTime, dex),
                                  std::nullopt, std::nullopt, correlationId);
    }

    void WebsocketApi::clearinghouseState(const std::string& user, const std::optional<std::string>& dex,
                                          std::optional<uint64_t> correlationId)
    {
        return impl_->signAndSend(RestEndpointType::ClearinghouseState,
                                  InfoRequestBuilder::clearinghouseState(user, dex),
                                  std::nullopt, std::nullopt, correlationId);
    }

    void WebsocketApi::userRateLimit(const std::string& user, std::optional<uint64_t> correlationId)
    {
        return impl_->signAndSend(RestEndpointType::UserRateLimit, InfoRequestBuilder::userRateLimit(user),
                                  std::nullopt, std::nullopt, correlationId);
    }

    void WebsocketApi::metaAndAssetCtxs(const std::optional<std::string>& dex, std::optional<uint64_t> correlationId)
    {
        return impl_->signAndSend(RestEndpointType::MetaAndAssetCtxs, InfoRequestBuilder::metaAndAssetCtxs(dex),
                                  std::nullopt, std::nullopt, correlationId);
    }

    void WebsocketApi::spotMetaAndAssetCtxs(std::optional<uint64_t> correlationId)
    {
        return impl_->signAndSend(RestEndpointType::SpotMetaAndAssetCtxs, InfoRequestBuilder::spotMetaAndAssetCtxs(),
                                  std::nullopt, std::nullopt, correlationId);
    }

    void WebsocketApi::spotClearinghouseState(const std::string& user, const std::optional<std::string>& dex,
                                              std::optional<uint64_t> correlationId)
    {
        return impl_->signAndSend(RestEndpointType::SpotClearinghouseState,
                                  InfoRequestBuilder::spotClearinghouseState(user, dex),
                                  std::nullopt, std::nullopt, correlationId);
    }

    void WebsocketApi::spotDeployState(const std::string& user, std::optional<uint64_t> correlationId)
    {
        return impl_->signAndSend(RestEndpointType::SpotDeployState, InfoRequestBuilder::spotDeployState(user),
                                  std::nullopt, std::nullopt, correlationId);
    }

    void WebsocketApi::spotPairDeployAuctionStatus(std::optional<uint64_t> correlationId)
    {
        return impl_->signAndSend(RestEndpointType::SpotPairDeployAuctionStatus,
                                  InfoRequestBuilder::spotPairDeployAuctionStatus(),
                                  std::nullopt, std::nullopt, correlationId);
    }

    void WebsocketApi::frontendOpenOrders(const std::string& user, const std::optional<std::string>& dex,
                                          std::optional<uint64_t> correlationId)
    {
        return impl_->signAndSend(RestEndpointType::FrontendOpenOrders,
                                  InfoRequestBuilder::frontendOpenOrders(user, dex),
                                  std::nullopt, std::nullopt, correlationId);
    }

    void WebsocketApi::historicalOrders(const std::string& user, std::optional<uint64_t> correlationId)
    {
        return impl_->signAndSend(RestEndpointType::HistoricalOrders, InfoRequestBuilder::historicalOrders(user),
                                  std::nullopt, std::nullopt, correlationId);
    }

    void WebsocketApi::userTwapSliceFills(const std::string& user, std::optional<uint64_t> correlationId)
    {
        return impl_->signAndSend(RestEndpointType::UserTwapSliceFills,
                                  InfoRequestBuilder::userTwapSliceFills(user),
                                  std::nullopt, std::nullopt, correlationId);
    }

    void WebsocketApi::subAccounts(const std::string& user, std::optional<uint64_t> correlationId)
    {
        return impl_->signAndSend(RestEndpointType::SubAccounts, InfoRequestBuilder::subAccounts(user),
                                  std::nullopt, std::nullopt, correlationId);
    }

    void WebsocketApi::userFees(const std::string& user, std::optional<uint64_t> correlationId)
    {
        return impl_->signAndSend(RestEndpointType::UserFees, InfoRequestBuilder::userFees(user),
                                  std::nullopt, std::nullopt, correlationId);
    }

    void WebsocketApi::maxBuilderFee(const std::string& user, const std::string& builder,
                                     std::optional<uint64_t> correlationId)
    {
        return impl_->signAndSend(RestEndpointType::MaxBuilderFee, InfoRequestBuilder::maxBuilderFee(user, builder),
                                  std::nullopt, std::nullopt, correlationId);
    }

    void WebsocketApi::approvedBuilders(const std::string& user, std::optional<uint64_t> correlationId)
    {
        return impl_->signAndSend(RestEndpointType::ApprovedBuilders, InfoRequestBuilder::approvedBuilders(user),
                                  std::nullopt, std::nullopt, correlationId);
    }

    void WebsocketApi::delegations(const std::string& user, std::optional<uint64_t> correlationId)
    {
        return impl_->signAndSend(RestEndpointType::Delegations, InfoRequestBuilder::delegations(user),
                                  std::nullopt, std::nullopt, correlationId);
    }

    void WebsocketApi::delegatorSummary(const std::string& user, std::optional<uint64_t> correlationId)
    {
        return impl_->signAndSend(RestEndpointType::DelegatorSummary, InfoRequestBuilder::delegatorSummary(user),
                                  std::nullopt, std::nullopt, correlationId);
    }

    void WebsocketApi::delegatorHistory(const std::string& user, std::optional<uint64_t> correlationId)
    {
        return impl_->signAndSend(RestEndpointType::DelegatorHistory, InfoRequestBuilder::delegatorHistory(user),
                                  std::nullopt, std::nullopt, correlationId);
    }

    void WebsocketApi::delegatorRewards(const std::string& user, std::optional<uint64_t> correlationId)
    {
        return impl_->signAndSend(RestEndpointType::DelegatorRewards, InfoRequestBuilder::delegatorRewards(user),
                                  std::nullopt, std::nullopt, correlationId);
    }

    void WebsocketApi::vaultDetails(const std::string& vaultAddress, const std::optional<std::string>& user,
                                    std::optional<uint64_t> correlationId)
    {
        return impl_->signAndSend(RestEndpointType::VaultDetails, InfoRequestBuilder::vaultDetails(vaultAddress, user),
                                  std::nullopt, std::nullopt, correlationId);
    }

    void WebsocketApi::userVaultEquities(const std::string& user, std::optional<uint64_t> correlationId)
    {
        return impl_->signAndSend(RestEndpointType::UserVaultEquities, InfoRequestBuilder::userVaultEquities(user),
                                  std::nullopt, std::nullopt, correlationId);
    }

    void WebsocketApi::portfolio(const std::string& user, std::optional<uint64_t> correlationId)
    {
        return impl_->signAndSend(RestEndpointType::Portfolio, InfoRequestBuilder::portfolio(user),
                                  std::nullopt, std::nullopt, correlationId);
    }

    void WebsocketApi::referral(const std::string& user, std::optional<uint64_t> correlationId)
    {
        return impl_->signAndSend(RestEndpointType::Referral, InfoRequestBuilder::referral(user),
                                  std::nullopt, std::nullopt, correlationId);
    }

    void WebsocketApi::userRole(const std::string& user, std::optional<uint64_t> correlationId)
    {
        return impl_->signAndSend(RestEndpointType::UserRole, InfoRequestBuilder::userRole(user),
                                  std::nullopt, std::nullopt, correlationId);
    }

    void WebsocketApi::borrowLendUserState(const std::string& user, std::optional<uint64_t> correlationId)
    {
        return impl_->signAndSend(RestEndpointType::BorrowLendUserState,
                                  InfoRequestBuilder::borrowLendUserState(user),
                                  std::nullopt, std::nullopt, correlationId);
    }

    void WebsocketApi::borrowLendReserveState(int token, std::optional<uint64_t> correlationId)
    {
        return impl_->signAndSend(RestEndpointType::BorrowLendReserveState,
                                  InfoRequestBuilder::borrowLendReserveState(token),
                                  std::nullopt, std::nullopt, correlationId);
    }

    void WebsocketApi::allBorrowLendReserveStates(std::optional<uint64_t> correlationId)
    {
        return impl_->signAndSend(RestEndpointType::AllBorrowLendReserveStates,
                                  InfoRequestBuilder::allBorrowLendReserveStates(),
                                  std::nullopt, std::nullopt, correlationId);
    }

    void WebsocketApi::placeOrder(const std::vector<OrderRequest>& orders,
                                  Grouping grouping,
                                  const std::optional<Builder>& builder,
                                  std::optional<uint64_t> correlationId,
                                  const std::optional<std::string>& vaultAddress)
    {
        return impl_->signAndSend(RestEndpointType::PlaceOrder,
                                  impl_->exchangeRequestBuilder.placeOrder(orders, grouping, builder),
                                  vaultAddress, std::nullopt, correlationId);
    }

    void WebsocketApi::cancelOrder(const std::vector<CancelRequest>& cancels,
                                   std::optional<uint64_t> correlationId,
                                   const std::optional<std::string>& vaultAddress)
    {
        return impl_->signAndSend(RestEndpointType::CancelOrder,
                                  impl_->exchangeRequestBuilder.cancelOrder(cancels),
                                  vaultAddress, std::nullopt, correlationId);
    }

    void WebsocketApi::cancelOrderByCloid(const std::vector<CancelByCloidRequest>& cancels,
                                          std::optional<uint64_t> correlationId,
                                          const std::optional<std::string>& vaultAddress)
    {
        return impl_->signAndSend(RestEndpointType::CancelOrderByCloid,
                                  impl_->exchangeRequestBuilder.cancelOrderByCloid(cancels),
                                  vaultAddress, std::nullopt, correlationId);
    }

    void WebsocketApi::scheduleCancel(const std::optional<uint64_t>& time,
                                      std::optional<uint64_t> correlationId,
                                      const std::optional<std::string>& vaultAddress)
    {
        return impl_->signAndSend(RestEndpointType::ScheduleCancel,
                                  impl_->exchangeRequestBuilder.scheduleCancel(time),
                                  vaultAddress, std::nullopt, correlationId);
    }

    void WebsocketApi::modifyOrder(const ModifyRequest& modify,
                                   std::optional<uint64_t> correlationId,
                                   const std::optional<std::string>& vaultAddress)
    {
        return impl_->signAndSend(RestEndpointType::ModifyOrder,
                                  impl_->exchangeRequestBuilder.modifyOrder(modify),
                                  vaultAddress, std::nullopt, correlationId);
    }

    void WebsocketApi::batchModifyOrder(const std::vector<ModifyRequest>& modifies,
                                        std::optional<uint64_t> correlationId,
                                        const std::optional<std::string>& vaultAddress)
    {
        return impl_->signAndSend(RestEndpointType::BatchModifyOrder,
                                  impl_->exchangeRequestBuilder.batchModifyOrder(modifies),
                                  vaultAddress, std::nullopt, correlationId);
    }
}
