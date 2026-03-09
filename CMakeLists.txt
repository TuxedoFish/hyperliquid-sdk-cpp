cmake_minimum_required(VERSION 3.16)
project(hyperliquid-sdk-cpp VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenSSL REQUIRED)
find_package(Boost REQUIRED COMPONENTS system)
find_package(simdjson CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

set(SDK_SOURCES
        src/rest/RestApiMessageParser.cpp
        src/rest/RestApi.cpp

        src/websocket/WebsocketRunner.cpp
        src/websocket/WebsocketApi.cpp
        src/websocket/WebsocketMessageParser.cpp

        src/signing/Signing.cpp
        src/signing/Signing.h

        include/hyperliquid/types/RequestTypes.h
        include/hyperliquid/signing/Wallet.h
)

add_library(hyperliquid-sdk STATIC ${SDK_SOURCES})

target_include_directories(hyperliquid-sdk
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(hyperliquid-sdk
        PRIVATE
        Boost::system
        OpenSSL::SSL
        OpenSSL::Crypto
        nlohmann_json::nlohmann_json
        simdjson::simdjson
)

if(NOT MSVC)
    target_compile_options(hyperliquid-sdk PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(hyperliquid-sdk PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        POSITION_INDEPENDENT_CODE ON
)

option(HYPERLIQUID_BUILD_EXAMPLES "Build example programs" OFF)

if(HYPERLIQUID_BUILD_EXAMPLES)
    add_executable(basic_book examples/basic_book.cpp)
    target_link_libraries(basic_book PRIVATE hyperliquid-sdk)
    add_executable(basic_orders examples/basic_orders.cpp)
    target_link_libraries(basic_orders PRIVATE hyperliquid-sdk)
    add_executable(basic_meta examples/basic_meta.cpp)
    target_link_libraries(basic_meta PRIVATE hyperliquid-sdk)
endif()
