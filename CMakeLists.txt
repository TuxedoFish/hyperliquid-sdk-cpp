cmake_minimum_required(VERSION 3.16)
project(hyperliquid-sdk-cpp VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenSSL REQUIRED)
find_package(Boost REQUIRED COMPONENTS system)
find_package(simdjson CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

include(FetchContent)
FetchContent_Declare(
    secp256k1
    GIT_REPOSITORY https://github.com/bitcoin-core/secp256k1.git
    GIT_TAG        v0.7.1
)
set(SECP256K1_ENABLE_MODULE_RECOVERY ON CACHE BOOL "" FORCE)
set(SECP256K1_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(SECP256K1_BUILD_EXHAUSTIVE_TESTS OFF CACHE BOOL "" FORCE)
set(SECP256K1_BUILD_BENCHMARK OFF CACHE BOOL "" FORCE)
set(SECP256K1_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SECP256K1_BUILD_CTIME_TESTS OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(secp256k1)

set(SDK_SOURCES
        src/rest/RestApiMessageParser.cpp
        src/rest/RestApi.cpp
        src/rest/HttpSession.cpp
        src/messages/InfoRequestBuilder.cpp
        src/messages/ExchangeRequestBuilder.cpp
        src/rest/SymbolMap.cpp
        src/config/Config.cpp

        src/websocket/WebsocketRunner.cpp
        src/websocket/WebsocketApi.cpp
        src/websocket/WebsocketMessageParser.cpp

        src/signing/Signing.cpp
        src/signing/SigningHelpers.cpp
        src/signing/Keys.cpp
        src/signing/sha3.c

        include/hyperliquid/types/RequestTypes.h
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
        secp256k1
        spdlog::spdlog
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
option(HYPERLIQUID_BUILD_TESTS "Build tests" OFF)

if(HYPERLIQUID_BUILD_TESTS)
    enable_testing()
    find_package(GTest CONFIG REQUIRED)

    function(hyperliquid_add_test NAME)
        add_executable(${NAME} tests/${NAME}.cpp)
        target_link_libraries(${NAME} PRIVATE hyperliquid-sdk GTest::gtest GTest::gtest_main nlohmann_json::nlohmann_json)
        target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
        add_test(NAME ${NAME} COMMAND ${NAME})
    endfunction()

    hyperliquid_add_test(signing_test)
    hyperliquid_add_test(rest_response_test)
    hyperliquid_add_test(rest_info_test)
    hyperliquid_add_test(leverage_margin_test)
    hyperliquid_add_test(schedule_cancel_test)
    hyperliquid_add_test(websocket_parser_test)
    hyperliquid_add_test(approve_agent_test)
    hyperliquid_add_test(user_rate_limit_test)
    hyperliquid_add_test(exchange_request_builder_test)
    hyperliquid_add_test(signing_helpers_test)
    hyperliquid_add_test(symbol_map_test)
endif()

if(HYPERLIQUID_BUILD_EXAMPLES)
    file(GLOB EXAMPLE_SOURCES examples/*.cpp)
    foreach(EXAMPLE_SOURCE ${EXAMPLE_SOURCES})
        get_filename_component(EXAMPLE_NAME ${EXAMPLE_SOURCE} NAME_WE)
        add_executable(${EXAMPLE_NAME} ${EXAMPLE_SOURCE})
        target_link_libraries(${EXAMPLE_NAME} PRIVATE hyperliquid-sdk nlohmann_json::nlohmann_json spdlog::spdlog)
        target_compile_definitions(${EXAMPLE_NAME} PRIVATE EXAMPLES_DIR="${CMAKE_CURRENT_SOURCE_DIR}/examples/")
    endforeach()
endif()
