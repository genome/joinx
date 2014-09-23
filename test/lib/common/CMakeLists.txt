cmake_minimum_required(VERSION 2.8)

set(TEST_LIBS common)
include_directories(${GTEST_INCLUDE_DIRS})

set(TEST_SOURCES
    TestCigarString.cpp
    TestCoordinateView.cpp
    TestInteger.cpp
    TestIub.cpp
    TestLocusCompare.cpp
    TestMutationSpectrum.cpp
    TestRegion.cpp
    TestSequence.cpp
    TestString.cpp
    TestStringView.cpp
    TestTokenizer.cpp
    )

add_unit_tests(TestCommon ${TEST_SOURCES})
