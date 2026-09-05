#include "../messages/ExchangeRequestBuilder.h"

#include <cmath>
#include <cstdio>
#include <stdexcept>
#include <string>
#include <unordered_map>

#include "InfoRequestBuilder.h"

namespace hyperliquid
{
    void ExchangeRequestBuilder::initializeMapping(const ApiConfig& config, RestApi* api)
    {
        if (!config.wallet.has_value())
        {
            // Symbol map only needed for authenticated responses
            return;
        }

        auto defaultMetaResponse = api->meta();
        int index = 0;
        for (const auto& asset : defaultMetaResponse.universe)
        {
            symbolMap_.set(asset.name, index);
            index++;
        }

        // Spot asset ids are 10000 + a pair's own `index` field - NOT its position in this
        // universe array. The two commonly diverge (this array is a curated listing; `index` is
        // the pair's real identity, e.g. what l2Book/assetCtxs call "coin" and what
        // assetCtxs is itself keyed by). Confirmed against live testnet data: joining
        // universe[i] to assetCtxs by array position matched ~4 pairs out of 1300+; joining by
        // `index` field matched all of them. Every pair also gets a "BASE/QUOTE" convenience
        // alias (first pair to claim a given string wins, regardless of canonical status) - but
        // a pair's own real name always wins over an alias, even one registered earlier, so
        // `set` (unconditional) is used for real names and `add` (first-wins) only for aliases.
        auto spotMetaResponse = api->spotMetaAndAssetCtxs();
        std::unordered_map<int, const SpotAssetMeta*> tokenByIndex;
        for (const auto& token : spotMetaResponse.meta.tokens)
        {
            tokenByIndex[token.index] = &token;
        }
        for (const auto& pair : spotMetaResponse.meta.universe)
        {
            int assetId = pair.index + 10000;
            symbolMap_.set(pair.name, assetId);
            if (pair.tokens.size() == 2)
            {
                const auto* baseToken = tokenByIndex.at(pair.tokens[0]);
                const auto* quoteToken = tokenByIndex.at(pair.tokens[1]);
                symbolMap_.add(baseToken->name + "/" + quoteToken->name, assetId);
            }
        }

        if (config.dexes.empty()) return;

        auto dexesResponse = api->perpDexs();
        int perpIdx = 1;
        for (const auto& dex : dexesResponse.dexes)
        {
            if (config.dexes.count(dex.name) == 0)
            {
                perpIdx++;
                continue;
            }

            auto dexMetaResponse = api->meta(dex.name);
            index = 0;
            for (const auto& asset : dexMetaResponse.universe)
            {
                symbolMap_.set(asset.name, 100000 + (perpIdx * 10000) + index);
                index++;
            }
            perpIdx++;
        }
    }


    // Matches Python SDK's float_to_wire: Decimal(f"{x:.8f}").normalize() formatted with :f
    static std::string floatToWire(double x)
    {
        char buf[64];
        std::snprintf(buf, sizeof(buf), "%.8f", x);
        std::string s(buf);

        if (std::abs(std::stod(s) - x) >= 1e-12)
            throw std::invalid_argument("floatToWire causes rounding: " + std::to_string(x));

        // Normalize: strip trailing zeros, then trailing dot
        auto dot = s.find('.');
        if (dot != std::string::npos)
        {
            auto last = s.find_last_not_of('0');
            if (last == dot)
                s = s.substr(0, dot); // integer, drop the dot
            else
                s = s.substr(0, last + 1);
        }
        return s;
    }

    nlohmann::ordered_json ExchangeRequestBuilder::buildOrderWire(const OrderRequest& order) const
    {
        nlohmann::ordered_json orderJson;
        orderJson["a"] = order.assetId
            ? *order.assetId
            : symbolMap_.resolve(order.asset);
        orderJson["b"] = order.isBuy;
        orderJson["p"] = floatToWire(order.price);
        orderJson["s"] = floatToWire(order.size);
        orderJson["r"] = order.reduceOnly;

        if (order.limit)
        {
            nlohmann::ordered_json limitInner;
            limitInner["tif"] = toString(order.limit->tif);
            nlohmann::ordered_json limitOuter;
            limitOuter["limit"] = limitInner;
            orderJson["t"] = limitOuter;
        }
        else if (order.trigger)
        {
            nlohmann::ordered_json triggerInner;
            triggerInner["isMarket"] = order.trigger->isMarket;
            triggerInner["triggerPx"] = floatToWire(order.trigger->triggerPx);
            triggerInner["tpsl"] = toString(order.trigger->tpsl);
            nlohmann::ordered_json triggerOuter;
            triggerOuter["trigger"] = triggerInner;
            orderJson["t"] = triggerOuter;
        }

        if (order.cloid) orderJson["c"] = *order.cloid;

        return orderJson;
    }

    nlohmann::ordered_json ExchangeRequestBuilder::placeOrder(const std::vector<OrderRequest>& orders,
                                                              Grouping grouping,
                                                              const std::optional<Builder>& builder) const
    {
        nlohmann::ordered_json ordersJson = nlohmann::ordered_json::array();
        for (const auto& order : orders)
            ordersJson.push_back(buildOrderWire(order));

        nlohmann::ordered_json action;
        action["type"] = "order";
        action["orders"] = ordersJson;
        action["grouping"] = toString(grouping);

        if (builder)
        {
            nlohmann::ordered_json builderJson;
            builderJson["b"] = builder->address;
            builderJson["f"] = builder->fee;
            action["builder"] = builderJson;
        }

        nlohmann::ordered_json body;
        body["action"] = action;
        return body;
    }

    nlohmann::ordered_json ExchangeRequestBuilder::cancelOrder(const std::vector<CancelRequest>& cancels) const
    {
        nlohmann::ordered_json cancelsJson = nlohmann::ordered_json::array();
        for (const auto& cancel : cancels)
        {
            nlohmann::ordered_json cancelJson;
            cancelJson["a"] = cancel.assetId
                ? *cancel.assetId
                : symbolMap_.resolve(cancel.asset);
            cancelJson["o"] = cancel.oid;
            cancelsJson.push_back(cancelJson);
        }

        nlohmann::ordered_json action;
        action["type"] = "cancel";
        action["cancels"] = cancelsJson;

        nlohmann::ordered_json body;
        body["action"] = action;
        return body;
    }

    nlohmann::ordered_json ExchangeRequestBuilder::cancelOrderByCloid(
        const std::vector<CancelByCloidRequest>& cancels) const
    {
        nlohmann::ordered_json cancelsJson = nlohmann::ordered_json::array();
        for (const auto& cancel : cancels)
        {
            nlohmann::ordered_json cancelJson;
            cancelJson["asset"] = cancel.assetId
                ? *cancel.assetId
                : symbolMap_.resolve(cancel.asset);
            cancelJson["cloid"] = cancel.cloid;
            cancelsJson.push_back(cancelJson);
        }

        nlohmann::ordered_json action;
        action["type"] = "cancelByCloid";
        action["cancels"] = cancelsJson;

        nlohmann::ordered_json body;
        body["action"] = action;
        return body;
    }

    nlohmann::ordered_json ExchangeRequestBuilder::scheduleCancel(const std::optional<uint64_t>& time) const
    {
        nlohmann::ordered_json action;
        action["type"] = "scheduleCancel";
        if (time) action["time"] = *time;

        nlohmann::ordered_json body;
        body["action"] = action;
        return body;
    }

    static void setModifyOid(nlohmann::ordered_json& json, const ModifyRequest& modify)
    {
        if (modify.oid)
            json["oid"] = *modify.oid;
        else if (modify.cloid)
            json["oid"] = *modify.cloid;
        else
            throw std::invalid_argument("ModifyRequest requires either oid or cloid");
    }

    nlohmann::ordered_json ExchangeRequestBuilder::modifyOrder(const ModifyRequest& modify) const
    {
        nlohmann::ordered_json action;
        action["type"] = "modify";
        setModifyOid(action, modify);
        action["order"] = buildOrderWire(modify.order);

        nlohmann::ordered_json body;
        body["action"] = action;
        return body;
    }

    nlohmann::ordered_json ExchangeRequestBuilder::batchModifyOrder(const std::vector<ModifyRequest>& modifies) const
    {
        nlohmann::ordered_json modifiesJson = nlohmann::ordered_json::array();
        for (const auto& modify : modifies)
        {
            nlohmann::ordered_json modifyJson;
            setModifyOid(modifyJson, modify);
            modifyJson["order"] = buildOrderWire(modify.order);
            modifiesJson.push_back(modifyJson);
        }

        nlohmann::ordered_json action;
        action["type"] = "batchModify";
        action["modifies"] = modifiesJson;

        nlohmann::ordered_json body;
        body["action"] = action;
        return body;
    }

    nlohmann::ordered_json ExchangeRequestBuilder::updateLeverage(const UpdateLeverageRequest& request) const
    {
        nlohmann::ordered_json action;
        action["type"] = "updateLeverage";
        action["asset"] = request.assetId
            ? *request.assetId
            : symbolMap_.resolve(request.asset);
        action["isCross"] = request.isCross;
        action["leverage"] = request.leverage;

        nlohmann::ordered_json body;
        body["action"] = action;
        return body;
    }

    nlohmann::ordered_json ExchangeRequestBuilder::updateIsolatedMargin(const UpdateIsolatedMarginRequest& request) const
    {
        nlohmann::ordered_json action;
        action["type"] = "updateIsolatedMargin";
        action["asset"] = request.assetId
            ? *request.assetId
            : symbolMap_.resolve(request.asset);
        action["isBuy"] = request.isBuy;
        action["ntli"] = request.ntli;

        nlohmann::ordered_json body;
        body["action"] = action;
        return body;
    }

    nlohmann::ordered_json ExchangeRequestBuilder::approveAgent(const ApproveAgentRequest& request) const
    {
        nlohmann::ordered_json action;
        action["type"] = "approveAgent";
        action["agentAddress"] = request.agentAddress;
        if (request.agentName) action["agentName"] = *request.agentName;

        nlohmann::ordered_json body;
        body["action"] = action;
        return body;
    }

    nlohmann::ordered_json ExchangeRequestBuilder::twapOrder(const TwapOrderRequest& request) const
    {
        nlohmann::ordered_json twap;
        twap["a"] = request.assetId
            ? *request.assetId
            : symbolMap_.resolve(request.asset);
        twap["b"] = request.isBuy;
        twap["s"] = floatToWire(request.size);
        twap["r"] = request.reduceOnly;
        twap["m"] = request.minutes;
        twap["t"] = request.randomize;

        nlohmann::ordered_json action;
        action["type"] = "twapOrder";
        action["twap"] = twap;

        nlohmann::ordered_json body;
        body["action"] = action;
        return body;
    }

    nlohmann::ordered_json ExchangeRequestBuilder::twapCancel(const TwapCancelRequest& request) const
    {
        nlohmann::ordered_json action;
        action["type"] = "twapCancel";
        action["a"] = request.assetId
            ? *request.assetId
            : symbolMap_.resolve(request.asset);
        action["t"] = request.twapId;

        nlohmann::ordered_json body;
        body["action"] = action;
        return body;
    }

    nlohmann::ordered_json ExchangeRequestBuilder::cDeposit(uint64_t wei) const
    {
        nlohmann::ordered_json action;
        action["type"] = "cDeposit";
        action["wei"] = wei;

        nlohmann::ordered_json body;
        body["action"] = action;
        return body;
    }

    nlohmann::ordered_json ExchangeRequestBuilder::cWithdraw(uint64_t wei) const
    {
        nlohmann::ordered_json action;
        action["type"] = "cWithdraw";
        action["wei"] = wei;

        nlohmann::ordered_json body;
        body["action"] = action;
        return body;
    }

    nlohmann::ordered_json ExchangeRequestBuilder::vaultTransfer(const VaultTransferRequest& request) const
    {
        nlohmann::ordered_json action;
        action["type"] = "vaultTransfer";
        action["vaultAddress"] = request.vaultAddress;
        action["isDeposit"] = request.isDeposit;
        // Unlike usdSend/spotSend (decimal strings), vaultTransfer is a plain L1 action whose
        // usd field mirrors USDC's own on-chain representation: raw integer units at USDC's
        // 6 decimals (vs. ETH's 18), so $5 is sent as 5_000_000.
        action["usd"] = static_cast<uint64_t>(std::llround(request.usd * 1e6));

        nlohmann::ordered_json body;
        body["action"] = action;
        return body;
    }

    nlohmann::ordered_json ExchangeRequestBuilder::usdClassTransfer(const UsdClassTransferRequest& request) const
    {
        nlohmann::ordered_json action;
        action["type"] = "usdClassTransfer";
        action["amount"] = floatToWire(request.amount);
        action["toPerp"] = request.toPerp;

        nlohmann::ordered_json body;
        body["action"] = action;
        return body;
    }

    nlohmann::ordered_json ExchangeRequestBuilder::sendAsset(const SendAssetRequest& request) const
    {
        nlohmann::ordered_json action;
        action["type"] = "sendAsset";
        action["destination"] = request.destination;
        action["sourceDex"] = request.sourceDex;
        action["destinationDex"] = request.destinationDex;
        action["token"] = request.token;
        action["amount"] = floatToWire(request.amount);
        action["fromSubAccount"] = request.fromSubAccount;

        nlohmann::ordered_json body;
        body["action"] = action;
        return body;
    }

    nlohmann::ordered_json ExchangeRequestBuilder::usdSend(const UsdSendRequest& request) const
    {
        nlohmann::ordered_json action;
        action["type"] = "usdSend";
        action["destination"] = request.destination;
        action["amount"] = floatToWire(request.amount);

        nlohmann::ordered_json body;
        body["action"] = action;
        return body;
    }

    nlohmann::ordered_json ExchangeRequestBuilder::spotSend(const SpotSendRequest& request) const
    {
        nlohmann::ordered_json action;
        action["type"] = "spotSend";
        action["destination"] = request.destination;
        action["token"] = request.token;
        action["amount"] = floatToWire(request.amount);

        nlohmann::ordered_json body;
        body["action"] = action;
        return body;
    }

    nlohmann::ordered_json ExchangeRequestBuilder::withdraw3(const Withdraw3Request& request) const
    {
        nlohmann::ordered_json action;
        action["type"] = "withdraw3";
        action["destination"] = request.destination;
        action["amount"] = floatToWire(request.amount);

        nlohmann::ordered_json body;
        body["action"] = action;
        return body;
    }

    nlohmann::ordered_json ExchangeRequestBuilder::approveBuilderFee(const ApproveBuilderFeeRequest& request) const
    {
        nlohmann::ordered_json action;
        action["type"] = "approveBuilderFee";
        action["maxFeeRate"] = request.maxFeeRate;
        action["builder"] = request.builder;

        nlohmann::ordered_json body;
        body["action"] = action;
        return body;
    }

    nlohmann::ordered_json ExchangeRequestBuilder::tokenDelegate(const TokenDelegateRequest& request) const
    {
        nlohmann::ordered_json action;
        action["type"] = "tokenDelegate";
        action["validator"] = request.validator;
        action["isUndelegate"] = request.isUndelegate;
        action["wei"] = request.wei;

        nlohmann::ordered_json body;
        body["action"] = action;
        return body;
    }
}
