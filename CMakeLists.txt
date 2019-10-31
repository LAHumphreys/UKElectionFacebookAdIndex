#
# Project Definition
#
cmake_minimum_required(VERSION 3.8.2)
project(ElectionDataAnal)

#
# Find dependencies
#
find_package(ThreadComms 0.1 REQUIRED)
find_package(FixedJSON 0.1 REQUIRED)
find_package(DevToolsLog 0.1 REQUIRED)
find_package(OSDevTools 0.1 REQUIRED)
find_package(UtilTime 0.1 REQUIRED)
find_package(GTKUtils 0.1 REQUIRED)
find_package(Boost 0.1 REQUIRED COMPONENTS system thread)
find_package(WEBSOCKETPP 0.8.1 REQUIRED)
find_package(CPPWebSocketResponseRequest 0.1 REQUIRED)
find_package(OpenSSL REQUIRED)


add_library(AdParser STATIC
    include/FacebookParser.h
    include/FacebookAd.h
    internal_includes/FacebookAdJSON.h
    src/FacebookParser.cpp
    src/FacebookAd.cpp
)

target_link_libraries(AdParser PUBLIC
    FixedJSON::FixedJSON
    DevToolsLog::Log
)

target_include_directories(AdParser PUBLIC
    $<BUILD_INTERFACE:${ElectionDataAnal_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(AdParser PRIVATE cxx_std_17)

set_property(TARGET AdParser PROPERTY PUBLIC_HEADER
    ${ElectionDataAnal_SOURCE_DIR}/include/FacebookAd.h
)


#
# Test Configuration
#
find_package(GTest REQUIRED)

add_executable(ad_parser test/ad_parser.cpp internal_includes/test_utils.h)

enable_testing()
add_test(ad_parser ad_parser)

target_compile_features(ad_parser PRIVATE cxx_std_17)
target_link_libraries(ad_parser PUBLIC AdParser GTest::GTest GTest::Main)

#
# Configure Coverage Build
#
SET(CMAKE_CXX_FLAGS_COVERAGE
    "${GCC_DEBUG_FLAGS} -fprofile-arcs -ftest-coverage"
    CACHE STRING "Flags used by the C++ compiler during coverage builds."
    FORCE )
SET(CMAKE_C_FLAGS_COVERAGE
    "${GCC_DEBUG_FLAGS} -fprofile-arcs -ftest-coverage"
    CACHE STRING "Flags used by the C compiler during coverage builds."
    FORCE )
MARK_AS_ADVANCED(
    CMAKE_CXX_FLAGS_COVERAGE
    CMAKE_C_FLAGS_COVERAGE)
