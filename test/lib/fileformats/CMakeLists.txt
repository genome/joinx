cmake_minimum_required(VERSION 2.8)

set(TEST_LIBS fileformats ${Boost_LIBRARIES})
include_directories(${GTEST_INCLUDE_DIRS})

def_test(Bed)
def_test(BedStream)
def_test(ReferenceSequence)
def_test(TranscriptStructure)
def_test(Variant)
def_test(VcfEntry)
def_test(VcfHeader)
def_test(VcfMap)
def_test(VcfValidator)
