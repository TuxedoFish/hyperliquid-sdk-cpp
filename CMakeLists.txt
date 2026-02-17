cmake_minimum_required(VERSION 3.16)
project(hyperliquid-sdk-cpp VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenSSL REQUIRED)
find_package(Boost REQUIRED COMPONENTS system)
find_package(simdjson CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

set(SDK_SOURCES
        src/websocket/WSRunner.cpp
        src/websocket/MarketData.cpp
        src/websocket/WSMessageParser.cpp
        src/rest/RestMessageParser.cpp
        src/rest/InfoApi.cpp
        include/hyperliquid/types/RequestTypes.h
)

add_library(hyperliquid-sdk STATIC ${SDK_SOURCES})

target_include_directories(hyperliquid-sdk
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/src/websocket
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
    add_executable(print_book examples/print_book.cpp)
    target_link_libraries(print_book PRIVATE hyperliquid-sdk)

    add_executable(fetch_meta examples/fetch_meta.cpp)
    target_link_libraries(fetch_meta PRIVATE hyperliquid-sdk)
endif()
