cmake_minimum_required(VERSION 2.8)

set(TEST_LIBS common)
include_directories(${GTEST_INCLUDE_DIRS})

set(TEST_SOURCES
    TestCigarString.cpp
    TestIub.cpp
    TestStringView.cpp
    TestMutationSpectrum.cpp
    TestRegion.cpp
    TestSequence.cpp
    TestTokenizer.cpp
    )

add_unit_tests(TestCommon ${TEST_SOURCES})
