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

    void WebsocketApi::updateLeverage(const UpdateLeverageRequest& request,
                                      std::optional<uint64_t> correlationId,
                                      const std::optional<std::string>& vaultAddress)
    {
        return impl_->signAndSend(RestEndpointType::UpdateLeverage,
                                  impl_->exchangeRequestBuilder.updateLeverage(request),
                                  vaultAddress, std::nullopt, correlationId);
    }

    void WebsocketApi::updateIsolatedMargin(const UpdateIsolatedMarginRequest& request,
                                            std::optional<uint64_t> correlationId,
                                            const std::optional<std::string>& vaultAddress)
    {
        return impl_->signAndSend(RestEndpointType::UpdateIsolatedMargin,
                                  impl_->exchangeRequestBuilder.updateIsolatedMargin(request),
                                  vaultAddress, std::nullopt, correlationId);
    }

    void WebsocketApi::approveAgent(const ApproveAgentRequest& request,
                                    std::optional<uint64_t> correlationId,
                                    const std::optional<std::string>& vaultAddress)
    {
        return impl_->signAndSend(RestEndpointType::ApproveAgent,
                                  impl_->exchangeRequestBuilder.approveAgent(request),
                                  vaultAddress, std::nullopt, correlationId);
    }

    void WebsocketApi::agentSetAbstraction(UserAbstractionMode abstraction,
                                           std::optional<uint64_t> correlationId,
                                           const std::optional<std::string>& vaultAddress)
    {
        return impl_->signAndSend(RestEndpointType::AgentSetAbstraction,
                                  impl_->exchangeRequestBuilder.agentSetAbstraction(abstraction),
                                  vaultAddress, std::nullopt, correlationId);
    }

    void WebsocketApi::twapOrder(const TwapOrderRequest& request,
                                 std::optional<uint64_t> correlationId,
                                 const std::optional<std::string>& vaultAddress)
    {
        return impl_->signAndSend(RestEndpointType::TwapOrder,
                                  impl_->exchangeRequestBuilder.twapOrder(request),
                                  vaultAddress, std::nullopt, correlationId);
    }

    void WebsocketApi::twapCancel(const TwapCancelRequest& request,
                                  std::optional<uint64_t> correlationId,
                                  const std::optional<std::string>& vaultAddress)
    {
        return impl_->signAndSend(RestEndpointType::TwapCancel,
                                  impl_->exchangeRequestBuilder.twapCancel(request),
                                  vaultAddress, std::nullopt, correlationId);
    }

    void WebsocketApi::vaultTransfer(const VaultTransferRequest& request,
                                     std::optional<uint64_t> correlationId,
                                     const std::optional<std::string>& vaultAddress)
    {
        return impl_->signAndSend(RestEndpointType::VaultTransfer,
                                  impl_->exchangeRequestBuilder.vaultTransfer(request),
                                  vaultAddress, std::nullopt, correlationId);
    }

    void WebsocketApi::hip3LiquidatorTransfer(const Hip3LiquidatorTransferRequest& request,
                                              std::optional<uint64_t> correlationId,
                                              const std::optional<std::string>& vaultAddress)
    {
        return impl_->signAndSend(RestEndpointType::Hip3LiquidatorTransfer,
                                  impl_->exchangeRequestBuilder.hip3LiquidatorTransfer(request),
                                  vaultAddress, std::nullopt, correlationId);
    }

    void WebsocketApi::usdClassTransfer(const UsdClassTransferRequest& request,
                                        std::optional<uint64_t> correlationId,
                                        const std::optional<std::string>& vaultAddress)
    {
        return impl_->signAndSend(RestEndpointType::UsdClassTransfer,
                                  impl_->exchangeRequestBuilder.usdClassTransfer(request),
                                  vaultAddress, std::nullopt, correlationId);
    }

    void WebsocketApi::sendAsset(const SendAssetRequest& request,
                                 std::optional<uint64_t> correlationId,
                                 const std::optional<std::string>& vaultAddress)
    {
        return impl_->signAndSend(RestEndpointType::SendAsset,
                                  impl_->exchangeRequestBuilder.sendAsset(request),
                                  vaultAddress, std::nullopt, correlationId);
    }

    void WebsocketApi::usdSend(const UsdSendRequest& request,
                               std::optional<uint64_t> correlationId,
                               const std::optional<std::string>& vaultAddress)
    {
        return impl_->signAndSend(RestEndpointType::UsdSend,
                                  impl_->exchangeRequestBuilder.usdSend(request),
                                  vaultAddress, std::nullopt, correlationId);
    }

    void WebsocketApi::spotSend(const SpotSendRequest& request,
                                std::optional<uint64_t> correlationId,
                                const std::optional<std::string>& vaultAddress)
    {
        return impl_->signAndSend(RestEndpointType::SpotSend,
                                  impl_->exchangeRequestBuilder.spotSend(request),
                                  vaultAddress, std::nullopt, correlationId);
    }

    void WebsocketApi::withdraw3(const Withdraw3Request& request,
                                 std::optional<uint64_t> correlationId,
                                 const std::optional<std::string>& vaultAddress)
    {
        return impl_->signAndSend(RestEndpointType::Withdraw3,
                                  impl_->exchangeRequestBuilder.withdraw3(request),
                                  vaultAddress, std::nullopt, correlationId);
    }

    void WebsocketApi::approveBuilderFee(const ApproveBuilderFeeRequest& request,
                                         std::optional<uint64_t> correlationId,
                                         const std::optional<std::string>& vaultAddress)
    {
        return impl_->signAndSend(RestEndpointType::ApproveBuilderFee,
                                  impl_->exchangeRequestBuilder.approveBuilderFee(request),
                                  vaultAddress, std::nullopt, correlationId);
    }

    void WebsocketApi::userSetAbstraction(const UserSetAbstractionRequest& request,
                                          std::optional<uint64_t> correlationId,
                                          const std::optional<std::string>& vaultAddress)
    {
        return impl_->signAndSend(RestEndpointType::UserSetAbstraction,
                                  impl_->exchangeRequestBuilder.userSetAbstraction(request),
                                  vaultAddress, std::nullopt, correlationId);
    }

    void WebsocketApi::cDeposit(uint64_t wei,
                                std::optional<uint64_t> correlationId,
                                const std::optional<std::string>& vaultAddress)
    {
        return impl_->signAndSend(RestEndpointType::CDeposit,
                                  impl_->exchangeRequestBuilder.cDeposit(wei),
                                  vaultAddress, std::nullopt, correlationId);
    }

    void WebsocketApi::cWithdraw(uint64_t wei,
                                 std::optional<uint64_t> correlationId,
                                 const std::optional<std::string>& vaultAddress)
    {
        return impl_->signAndSend(RestEndpointType::CWithdraw,
                                  impl_->exchangeRequestBuilder.cWithdraw(wei),
                                  vaultAddress, std::nullopt, correlationId);
    }

    void WebsocketApi::tokenDelegate(const TokenDelegateRequest& request,
                                     std::optional<uint64_t> correlationId,
                                     const std::optional<std::string>& vaultAddress)
    {
        return impl_->signAndSend(RestEndpointType::TokenDelegate,
                                  impl_->exchangeRequestBuilder.tokenDelegate(request),
                                  vaultAddress, std::nullopt, correlationId);
    }

    void WebsocketApi::sendToEvmWithData(const SendToEvmWithDataRequest& request,
                                         std::optional<uint64_t> correlationId,
                                         const std::optional<std::string>& vaultAddress)
    {
        return impl_->signAndSend(RestEndpointType::SendToEvmWithData,
                                  impl_->exchangeRequestBuilder.sendToEvmWithData(request),
                                  vaultAddress, std::nullopt, correlationId);
    }

    void WebsocketApi::userDexAbstraction(const UserDexAbstractionRequest& request,
                                          std::optional<uint64_t> correlationId,
                                          const std::optional<std::string>& vaultAddress)
    {
        return impl_->signAndSend(RestEndpointType::UserDexAbstraction,
                                  impl_->exchangeRequestBuilder.userDexAbstraction(request),
                                  vaultAddress, std::nullopt, correlationId);
    }

    void WebsocketApi::agentSendAsset(const AgentSendAssetRequest& request,
                                      std::optional<uint64_t> correlationId,
                                      const std::optional<std::string>& vaultAddress)
    {
        return impl_->signAndSend(RestEndpointType::AgentSendAsset,
                                  impl_->exchangeRequestBuilder.agentSendAsset(request),
                                  vaultAddress, std::nullopt, correlationId);
    }

    void WebsocketApi::reserveRequestWeight(const ReserveRequestWeightRequest& request,
                                            std::optional<uint64_t> correlationId,
                                            const std::optional<std::string>& vaultAddress)
    {
        return impl_->signAndSend(RestEndpointType::ReserveRequestWeight,
                                  impl_->exchangeRequestBuilder.reserveRequestWeight(request),
                                  vaultAddress, std::nullopt, correlationId);
    }

    void WebsocketApi::noop(std::optional<uint64_t> correlationId,
                            const std::optional<std::string>& vaultAddress)
    {
        return impl_->signAndSend(RestEndpointType::Noop,
                                  impl_->exchangeRequestBuilder.noop(),
                                  vaultAddress, std::nullopt, correlationId);
    }
}
