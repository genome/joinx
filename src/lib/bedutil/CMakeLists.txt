cmake_minimum_required(VERSION 2.8)

project(bedutil)

set(SOURCES
    BedDeduplicator.hpp
    ConcordanceQuality.cpp
    ConcordanceQuality.hpp
    Intersect.hpp
    IntersectionOutputFormatter.cpp
    IntersectionOutputFormatter.hpp
    MergeSorted.hpp
    NoReferenceFilter.hpp
    RemappedContig.hpp
    SnvComparator.hpp
    SnvConcordance.cpp
    SnvConcordance.hpp
    Sort.hpp
    TypeFilter.hpp
)

add_library(bedutil ${SOURCES})
target_link_libraries(bedutil common fileformats ${Boost_LIBRARIES})
