cmake_minimum_required(VERSION 3.30)

add_executable(GW2TacO_Tests
    math_test.cpp
)

target_link_libraries(GW2TacO_Tests
    PRIVATE
        gtest_main
        taco_base
)
use_props(GW2TacO_Tests "${CMAKE_CONFIGURATION_TYPES}" "${DEFAULT_CXX_PROPS}")

include(GoogleTest)
gtest_discover_tests(GW2TacO_Tests)
