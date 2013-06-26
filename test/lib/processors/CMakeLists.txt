cmake_minimum_required(VERSION 2.8)

set(TEST_LIBS processors)
include_directories(${GTEST_INCLUDE_DIRS})

set(TEST_SOURCES
    TestBedDeduplicator.cpp
    TestIntersectFull.cpp
    TestMergeSorted.cpp
    TestNoReferenceFilter.cpp
    TestSnvComparator.cpp
    TestSort.cpp
    TestVariantContig.cpp
)

add_unit_tests(TestProcessors ${TEST_SOURCES})
