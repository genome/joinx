cmake_minimum_required(VERSION 2.8)

set(TEST_LIBS processors fileformats io common)
include_directories(${GTEST_INCLUDE_DIRS})

set(TEST_SOURCES
    TestBedDeduplicator.cpp
    TestGroupOverlapping.cpp
    TestIntersectFull.cpp
    TestMergeSorted.cpp
    TestRefStats.cpp
    TestSort.cpp
    TestVariantContig.cpp
    TestVcfGenotypeMatcher.cpp
)

add_unit_tests(TestProcessors ${TEST_SOURCES})
