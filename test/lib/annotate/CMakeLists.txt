cmake_minimum_required(VERSION 2.6)

set(TEST_LIBS bedutil annotate)
include_directories(${GTEST_INCLUDE_DIRS})

def_test(TranscriptStructure)
def_test(Region)
def_test(Sequence)
