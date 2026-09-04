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
