#include "hyperliquid/rest/RestApiMessageParser.h"
#include <iostream>
#include <simdjson.h>

namespace hyperliquid
{
    struct RestApiMessageParser::Impl
    {
        RestEndpointListener& listener;
        simdjson::ondemand::parser parser;
        simdjson::padded_string padded;

        explicit Impl(RestEndpointListener& listener) : listener(listener) {}

        void parse(const std::string& message, RestEndpointType type)
        {
            switch (type)
            {
            case RestEndpointType::Meta:
                listener.onMeta(parseMeta(message));
                break;
            default:
                std::cerr << "RestMessageParser: unhandled InfoEndpointType: "
                          << toString(type) << std::endl;
                break;
            }
        }

        MetaResponse parseMeta(const std::string& message)
        {
            MetaResponse response;
            padded = simdjson::padded_string(message.data(), message.size());
            auto doc = parser.iterate(padded);

            try
            {
                auto universe = doc["universe"].get_array().value();
                for (auto entry : universe)
                {
                    auto obj = entry.get_object().value();
                    AssetMeta asset;
                    asset.name = std::string(obj["name"].get_string().value());
                    asset.szDecimals = static_cast<int>(obj["szDecimals"].get_int64().value());
                    asset.maxLeverage = static_cast<int>(obj["maxLeverage"].get_int64().value());
                    response.universe.push_back(std::move(asset));
                }
            }
            catch (const simdjson::simdjson_error& e)
            {
                std::cerr << "RestMessageParser: parse error in meta: " << e.what() << std::endl;
            }

            return response;
        }
    };

    RestApiMessageParser::RestApiMessageParser(RestEndpointListener& listener)
        : impl_(std::make_unique<Impl>(listener)) {}

    RestApiMessageParser::~RestApiMessageParser() = default;
    RestApiMessageParser::RestApiMessageParser(RestApiMessageParser&&) noexcept = default;
    RestApiMessageParser& RestApiMessageParser::operator=(RestApiMessageParser&&) noexcept = default;

    void RestApiMessageParser::parse(const std::string& message, RestEndpointType type)
    {
        impl_->parse(message, type);
    }
} // namespace hyperliquid
