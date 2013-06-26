cmake_minimum_required(VERSION 2.8)

set(TEST_LIBS reports)
include_directories(${GTEST_INCLUDE_DIRS})

set(TEST_SOURCES
    TestSnvConcordance.cpp
)

add_unit_tests(TestSnvConcordance ${TEST_SOURCES})
