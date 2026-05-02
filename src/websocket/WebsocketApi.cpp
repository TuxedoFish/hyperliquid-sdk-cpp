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
    struct WebsocketApi::Impl : internal::WSListener
    {
        internal::WebsocketRunner ws;
        WebsocketApiListener& listener;
        bool stopping = false;
        std::thread thread;
        ExchangeRequestBuilder exchangeRequestBuilder;
        const ApiConfig& config;
        std::atomic<int> postRequestCounter;
        std::unordered_map<uint64_t, RestEndpointType> postRequestIdToType;
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
                    auto it = postRequestIdToType.find(id);
                    if (it != postRequestIdToType.end())
                    {
                        auto type = it->second;
                        postRequestIdToType.erase(it);
                        std::string payloadStr(payload.value());
                        listener.onPostResponse(payloadStr, type);
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
                         const std::optional<uint64_t>& expiresAfter = std::nullopt)
        {
            auto payload= Signing::prepareBody(config, type, std::move(body), vaultAddress, expiresAfter);
            auto payloadType = isAuthenticated(type) ? "action" : "info";
            int postRequestId = postRequestCounter.fetch_add(1);
            postRequestIdToType[postRequestId] = type;
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

    void WebsocketApi::spotMeta()
    {
        return impl_->signAndSend(RestEndpointType::SpotMeta, InfoRequestBuilder::spotMeta());
    }

    void WebsocketApi::meta(const std::optional<std::string>& dex)
    {
        return impl_->signAndSend(RestEndpointType::Meta, InfoRequestBuilder::meta(dex));
    }

    void WebsocketApi::outcomeMeta()
    {
        return impl_->signAndSend(RestEndpointType::OutcomeMeta, InfoRequestBuilder::outcomeMeta());
    }

    void WebsocketApi::perpDexs()
    {
        return impl_->signAndSend(RestEndpointType::PerpDexs, InfoRequestBuilder::perpDexs());
    }

    void WebsocketApi::placeOrder(const std::vector<OrderRequest>& orders,
                                  Grouping grouping,
                                  const std::optional<Builder>& builder)
    {
        return impl_->signAndSend(RestEndpointType::PlaceOrder,
                                  impl_->exchangeRequestBuilder.placeOrder(orders, grouping, builder));
    }

    void WebsocketApi::cancelOrder(const std::vector<CancelRequest>& cancels)
    {
        return impl_->signAndSend(RestEndpointType::CancelOrder,
                                  impl_->exchangeRequestBuilder.cancelOrder(cancels));
    }

    void WebsocketApi::cancelOrderByCloid(const std::vector<CancelByCloidRequest>& cancels)
    {
        return impl_->signAndSend(RestEndpointType::CancelOrderByCloid,
                                  impl_->exchangeRequestBuilder.cancelOrderByCloid(cancels));
    }

    void WebsocketApi::modifyOrder(const ModifyRequest& modify)
    {
        return impl_->signAndSend(RestEndpointType::ModifyOrder,
                                  impl_->exchangeRequestBuilder.modifyOrder(modify));
    }

    void WebsocketApi::batchModifyOrder(const std::vector<ModifyRequest>& modifies)
    {
        return impl_->signAndSend(RestEndpointType::BatchModifyOrder,
                                  impl_->exchangeRequestBuilder.batchModifyOrder(modifies));
    }
}
