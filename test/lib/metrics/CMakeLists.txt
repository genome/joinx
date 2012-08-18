cmake_minimum_required(VERSION 2.8)

set(TEST_LIBS metrics fileformats ${ZLIB_LIBRARIES})
include_directories(${GTEST_INCLUDE_DIRS})

def_test(Metrics)
