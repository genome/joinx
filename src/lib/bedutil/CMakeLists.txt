cmake_minimum_required(VERSION 2.6)

project(bedutil)

set(SOURCES
    Bed.cpp
    Bed.hpp
    BedFilterBase.hpp
    BedStream.cpp
    BedStream.hpp
    ConcordanceQuality.hpp
    IResultCollector.hpp
    LineUtils.hpp
    NoReferenceFilter.hpp
    SnvIntersector.cpp
    SnvIntersector.hpp
    SortBed.cpp
    SortBed.hpp
    intconfig.hpp
)

add_library(bedutil ${SOURCES})
