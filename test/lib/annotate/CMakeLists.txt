cmake_minimum_required(VERSION 2.8)

set(TEST_LIBS fileformats io common)
include_directories(${GTEST_INCLUDE_DIRS})

set(TEST_SOURCES
    TestHomopolymerAnnotator.cpp
    )

add_unit_tests(TestAnnotate ${TEST_SOURCES})
