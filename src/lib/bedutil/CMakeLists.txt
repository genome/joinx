cmake_minimum_required(VERSION 2.6)

project(bedutil)

set(SOURCES
    ConcordanceQuality.cpp
    ConcordanceQuality.hpp
    IResultCollector.hpp
    Intersect.hpp
    IntersectBed.cpp
    IntersectBed.hpp
    NoReferenceFilter.hpp
    ResultStreamWriter.cpp
    ResultStreamWriter.hpp
    SnvComparator.cpp
    SnvComparator.hpp
    SortBed.cpp
    SortBed.hpp
    SortTempFile.hpp
    TypeFilter.hpp
)

add_library(bedutil ${SOURCES})
target_link_libraries(bedutil common fileformats)
