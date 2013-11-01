cmake_minimum_required(VERSION 2.8)

set(TEST_LIBS ui processors fileformats common)
include_directories(${GTEST_INCLUDE_DIRS})

set(TEST_SOURCES
    TestBedMergeCommand
)

add_unit_tests(TestUiLib ${TEST_SOURCES})
