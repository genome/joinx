cmake_minimum_required(VERSION 2.6)

set(TEST_LIBS fileformats ${Boost_LIBRARIES})
include_directories(${GTEST_INCLUDE_DIRS})

def_test(Bed)
def_test(BedStream)
def_test(TranscriptStructure)
def_test(ReferenceSequence)
