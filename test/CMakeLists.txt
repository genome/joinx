cmake_minimum_required(VERSION 2.6)

set(TEST_LIBS bed)
include_directories(${GTEST_INCLUDE_DIRS})

def_test(Bed)
def_test(LineUtils)
def_test(NoReferenceFilter)
def_test(BedStream)
