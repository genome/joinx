cmake_minimum_required(VERSION 2.6)

project(fileformats)

set(SOURCES
    Bed.cpp
    Bed.hpp
    BedFilterBase.hpp
    BedStream.cpp
    BedStream.hpp
    TranscriptStructure.cpp
    TranscriptStructure.hpp
)

add_library(fileformats ${SOURCES})
target_link_libraries(fileformats common)
