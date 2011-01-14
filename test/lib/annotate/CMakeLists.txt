cmake_minimum_required(VERSION 2.6)

set(TEST_LIBS bedutil annotation ${Boost_LIBRARIES})
include_directories(${GTEST_INCLUDE_DIRS})

def_test(ReferenceSequence)
