cmake_minimum_required(VERSION 3.16)
project(hyperliquid-sdk-cpp VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Off by default so local dev builds stay permissive mid-edit; CI turns this on so new
# warnings can't silently creep in.
option(HYPERLIQUID_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)
set(HYPERLIQUID_WARNING_FLAGS -Wall -Wextra -Wpedantic)
if(HYPERLIQUID_WARNINGS_AS_ERRORS)
    list(APPEND HYPERLIQUID_WARNING_FLAGS -Werror)
endif()

# Off by default - real runtime overhead, not something you want in every local build.
# CI runs a dedicated sanitizer job with this on.
option(HYPERLIQUID_SANITIZE "Build with AddressSanitizer + UndefinedBehaviorSanitizer" OFF)
set(HYPERLIQUID_SANITIZE_FLAGS -fsanitize=address,undefined -fno-omit-frame-pointer)

# Off by default - gcov instrumentation slows the build and bloats binaries for no benefit
# outside a dedicated coverage run. CI runs a dedicated coverage job with this on.
option(HYPERLIQUID_COVERAGE "Build with gcov coverage instrumentation" OFF)
set(HYPERLIQUID_COVERAGE_FLAGS --coverage -fno-inline -fno-inline-small-functions -fno-default-inline)

# Off by default - libFuzzer is a clang-only feature and these binaries are only ever
# meant for the dedicated scheduled fuzz workflow, not normal local/CI builds.
option(HYPERLIQUID_BUILD_FUZZERS "Build libFuzzer harnesses for the REST/websocket message parsers (clang only)" OFF)
# Applied to hyperliquid-sdk itself (PUBLIC) so the actual parsing code is coverage-instrumented
# and ASan-checked, not just the thin harness translation units in fuzz/. fuzzer-no-link gives
# coverage instrumentation without pulling in a libFuzzer main - only the harness executables do that.
set(HYPERLIQUID_FUZZER_LIB_FLAGS -fsanitize=fuzzer-no-link,address,undefined -fno-omit-frame-pointer -g)
set(HYPERLIQUID_FUZZER_EXE_FLAGS -fsanitize=fuzzer,address,undefined -fno-omit-frame-pointer -g)

include(GNUInstallDirs)

find_package(OpenSSL REQUIRED)
find_package(Boost REQUIRED COMPONENTS system)
find_package(simdjson CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(ZLIB REQUIRED)

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
        src/websocket/WebsocketParsingUtils.cpp

        src/signing/Signing.cpp
        src/signing/SigningHelpers.cpp
        src/signing/Keys.cpp
        src/signing/sha3.c

        include/hyperliquid/types/RequestTypes.h
)

add_library(hyperliquid-sdk STATIC ${SDK_SOURCES})

target_include_directories(hyperliquid-sdk
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(hyperliquid-sdk
        PUBLIC
        Boost::system
        OpenSSL::SSL
        OpenSSL::Crypto
        nlohmann_json::nlohmann_json
        simdjson::simdjson
        spdlog::spdlog
        ZLIB::ZLIB
        PRIVATE
        $<BUILD_INTERFACE:secp256k1_objs>
)

if(NOT MSVC)
    target_compile_options(hyperliquid-sdk PRIVATE ${HYPERLIQUID_WARNING_FLAGS})
endif()

if(HYPERLIQUID_SANITIZE)
    target_compile_options(hyperliquid-sdk PUBLIC ${HYPERLIQUID_SANITIZE_FLAGS})
    target_link_options(hyperliquid-sdk PUBLIC ${HYPERLIQUID_SANITIZE_FLAGS})
endif()

if(HYPERLIQUID_COVERAGE)
    target_compile_options(hyperliquid-sdk PUBLIC ${HYPERLIQUID_COVERAGE_FLAGS})
    target_link_options(hyperliquid-sdk PUBLIC --coverage)
endif()

set_target_properties(hyperliquid-sdk PROPERTIES
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED ON
        POSITION_INDEPENDENT_CODE ON
)

add_library(hyperliquid-sdk::hyperliquid-sdk ALIAS hyperliquid-sdk)

include(CMakePackageConfigHelpers)

install(TARGETS hyperliquid-sdk
        EXPORT hyperliquid-sdk-targets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(EXPORT hyperliquid-sdk-targets
        FILE hyperliquid-sdk-targets.cmake
        NAMESPACE hyperliquid-sdk::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/hyperliquid-sdk
)

configure_package_config_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/hyperliquid-sdk-config.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/hyperliquid-sdk-config.cmake
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/hyperliquid-sdk
)

write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/hyperliquid-sdk-config-version.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
)

install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/hyperliquid-sdk-config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/hyperliquid-sdk-config-version.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/hyperliquid-sdk
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
        if(NOT MSVC)
            target_compile_options(${NAME} PRIVATE ${HYPERLIQUID_WARNING_FLAGS})
        endif()
        add_test(NAME ${NAME} COMMAND ${NAME})
    endfunction()

    hyperliquid_add_test(signing_test)
    hyperliquid_add_test(rest_response_test)
    hyperliquid_add_test(rest_market_data_test)
    hyperliquid_add_test(rest_meta_test)
    hyperliquid_add_test(rest_account_test)
    hyperliquid_add_test(rest_delegation_test)
    hyperliquid_add_test(rest_borrow_lend_test)
    hyperliquid_add_test(rest_outcomes_test)
    hyperliquid_add_test(rest_deploy_test)
    hyperliquid_add_test(rest_vaults_test)
    hyperliquid_add_test(leverage_margin_test)
    hyperliquid_add_test(schedule_cancel_test)
    hyperliquid_add_test(hip3_liquidator_transfer_test)
    hyperliquid_add_test(websocket_parser_test)
    hyperliquid_add_test(approve_agent_test)
    hyperliquid_add_test(agent_set_abstraction_test)
    hyperliquid_add_test(user_rate_limit_test)
    hyperliquid_add_test(exchange_request_builder_test)
    hyperliquid_add_test(signing_helpers_test)
    hyperliquid_add_test(symbol_map_test)
    hyperliquid_add_test(transfers_test)
    hyperliquid_add_test(misc_info_test)
    hyperliquid_add_test(user_dex_abstraction_test)
endif()

if(HYPERLIQUID_BUILD_EXAMPLES)
    file(GLOB EXAMPLE_SOURCES examples/*.cpp)
    foreach(EXAMPLE_SOURCE ${EXAMPLE_SOURCES})
        get_filename_component(EXAMPLE_NAME ${EXAMPLE_SOURCE} NAME_WE)
        add_executable(${EXAMPLE_NAME} ${EXAMPLE_SOURCE})
        target_link_libraries(${EXAMPLE_NAME} PRIVATE hyperliquid-sdk nlohmann_json::nlohmann_json spdlog::spdlog)
        target_compile_definitions(${EXAMPLE_NAME} PRIVATE EXAMPLES_DIR="${CMAKE_CURRENT_SOURCE_DIR}/examples/")
        if(NOT MSVC)
            target_compile_options(${EXAMPLE_NAME} PRIVATE ${HYPERLIQUID_WARNING_FLAGS})
        endif()
    endforeach()
endif()

if(HYPERLIQUID_BUILD_FUZZERS)
    if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        message(FATAL_ERROR
            "HYPERLIQUID_BUILD_FUZZERS requires Clang (libFuzzer/-fsanitize=fuzzer is a "
            "clang-only feature). CMAKE_CXX_COMPILER_ID is '${CMAKE_CXX_COMPILER_ID}'. "
            "Reconfigure with -DCMAKE_CXX_COMPILER=clang++.")
    endif()

    target_compile_options(hyperliquid-sdk PUBLIC ${HYPERLIQUID_FUZZER_LIB_FLAGS})
    target_link_options(hyperliquid-sdk PUBLIC -fsanitize=address,undefined)

    function(hyperliquid_add_fuzzer NAME)
        add_executable(${NAME} fuzz/${NAME}.cpp)
        target_link_libraries(${NAME} PRIVATE hyperliquid-sdk)
        target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
        target_compile_options(${NAME} PRIVATE ${HYPERLIQUID_FUZZER_EXE_FLAGS})
        target_link_options(${NAME} PRIVATE ${HYPERLIQUID_FUZZER_EXE_FLAGS})
    endfunction()

    hyperliquid_add_fuzzer(rest_parser_fuzz)
    hyperliquid_add_fuzzer(websocket_parser_fuzz)
endif()
