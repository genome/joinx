cmake_minimum_required(VERSION 2.8)

set(TEST_LIBS fileformats io common)
include_directories(${GTEST_INCLUDE_DIRS})

set(TEST_SOURCES
    TestBed.cpp
    TestFasta.cpp
    TestInferFileType.cpp
    TestInputStream.cpp
    TestStreamFactory.cpp
    TestStreamHandler.cpp
    TestVariant.cpp
    TestVcfAlleleMerger.cpp
    TestVcfAltNormalizer.cpp
    TestVcfCompare.cpp
    TestVcfCustomType.cpp
    TestVcfCustomValue.cpp
    TestVcfEntry.cpp
    TestVcfEntryMerger.cpp
    TestVcfGenotypeCall.cpp
    TestVcfGenotypeMerger.cpp
    TestVcfHeader.cpp
    TestVcfMap.cpp
    TestVcfMatcher.cpp
    TestVcfMergeStrategy.cpp
    TestVcfRawVariant.cpp
    TestVcfReader.cpp
    TestVcfSampleTag.cpp
    TestVcfValueMergers.cpp
    TestWiggleReader.cpp
)
add_unit_tests(TestFileFormats ${TEST_SOURCES})
