cmake_minimum_required(VERSION 2.6)

set(TEST_LIBS bedutil)
include_directories(${GTEST_INCLUDE_DIRS})

def_test(NoReferenceFilter)
def_test(ResultStreamWriter)
def_test(SnvComparator)
