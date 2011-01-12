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
    IntersectBed.cpp
    IntersectBed.hpp
    LineUtils.hpp
    NoReferenceFilter.hpp
    ResultStreamWriter.cpp
    ResultStreamWriter.hpp
    SnvComparator.cpp
    SnvComparator.hpp
    SortBed.cpp
    SortBed.hpp
    SortTempFile.hpp
    TypeFilter.hpp
    intconfig.hpp
)

add_library(bedutil ${SOURCES})
