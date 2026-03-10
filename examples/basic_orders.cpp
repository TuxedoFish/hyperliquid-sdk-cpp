#include "test_config.h"

#include <hyperliquid/rest/RestApi.h>
#include <hyperliquid/rest/RestApiMessageParser.h>
#include <iostream>

int main()
{
    auto wallet = loadWalletFromConfig();
    hyperliquid::RestApi api(hyperliquid::Environment::Testnet, wallet);
    hyperliquid::RestApiMessageParser parser;

    hyperliquid::OrderRequest order;
    order.asset = "ETH";
    order.isBuy = true;
    order.price = 1800.0;
    order.size = 0.01;
    order.reduceOnly = false;
    order.limit = hyperliquid::LimitOrderType{hyperliquid::Tif::Gtc};

    std::cout << "Placing order on Hyperliquid testnet..." << std::endl;
    auto response = parser.parsePlaceOrder(
        api.placeOrder({order}, hyperliquid::Grouping::Na));

    std::cout << "Order response status: " << response.status << std::endl;
    for (const auto& status : response.statuses)
    {
        if (status.resting)
        {
            std::cout << "  Resting oid=" << status.resting->oid << std::endl;
        }
        else if (status.filled)
        {
            std::cout << "  Filled oid=" << status.filled->oid
                      << " avgPx=" << status.filled->avgPx
                      << " totalSz=" << status.filled->totalSz << std::endl;
        }
        else if (status.error)
        {
            std::cout << "  Error: " << *status.error << std::endl;
        }
    }

    return 0;
}
