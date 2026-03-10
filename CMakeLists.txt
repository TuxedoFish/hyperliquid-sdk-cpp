cmake_minimum_required(VERSION 3.16)
project(hyperliquid-sdk-cpp VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenSSL REQUIRED)
find_package(Boost REQUIRED COMPONENTS system)
find_package(simdjson CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(unofficial-secp256k1 CONFIG REQUIRED)

set(SDK_SOURCES
        src/rest/RestApiMessageParser.cpp
        src/rest/RestApi.cpp
        src/rest/HttpSession.cpp
        src/rest/InfoRequestBuilder.cpp
        src/rest/ExchangeRequestBuilder.cpp
        src/rest/SymbolMap.cpp

        src/websocket/WebsocketRunner.cpp
        src/websocket/WebsocketApi.cpp
        src/websocket/WebsocketMessageParser.cpp

        src/signing/Signing.cpp
        src/signing/sha3.c

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
        unofficial::secp256k1
        unofficial::secp256k1_precomputed
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

    add_executable(signing_test tests/signing_test.cpp)
    target_link_libraries(signing_test PRIVATE hyperliquid-sdk GTest::gtest GTest::gtest_main nlohmann_json::nlohmann_json)
    target_include_directories(signing_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
    add_test(NAME signing_test COMMAND signing_test)
endif()

if(HYPERLIQUID_BUILD_EXAMPLES)
    file(GLOB EXAMPLE_SOURCES examples/*.cpp)
    foreach(EXAMPLE_SOURCE ${EXAMPLE_SOURCES})
        get_filename_component(EXAMPLE_NAME ${EXAMPLE_SOURCE} NAME_WE)
        add_executable(${EXAMPLE_NAME} ${EXAMPLE_SOURCE})
        target_link_libraries(${EXAMPLE_NAME} PRIVATE hyperliquid-sdk nlohmann_json::nlohmann_json)
        target_compile_definitions(${EXAMPLE_NAME} PRIVATE EXAMPLES_DIR="${CMAKE_CURRENT_SOURCE_DIR}/examples/")
    endforeach()
endif()
