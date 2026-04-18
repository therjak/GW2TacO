cmake_minimum_required(VERSION 3.30)

add_executable(GW2TacO_Tests
    math_test.cpp
)

target_link_libraries(GW2TacO_Tests
    PRIVATE
        gtest_main
        taco_base
)

include(GoogleTest)
gtest_discover_tests(GW2TacO_Tests)
