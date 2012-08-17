cmake_minimum_required(VERSION 2.8)

set(TEST_LIBS processors)
include_directories(${GTEST_INCLUDE_DIRS})

def_test(BedDeduplicator)
def_test(IntersectFull)
def_test(MergeSorted)
def_test(NoReferenceFilter)
def_test(SnvComparator)
def_test(Sort)
def_test(VariantContig)