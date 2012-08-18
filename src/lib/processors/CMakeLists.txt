cmake_minimum_required(VERSION 2.8)

project(processors)

set(SOURCES
    BedDeduplicator.hpp
    IntersectFull.hpp
    IntersectSimple.hpp
    IntersectionOutputFormatter.cpp
    IntersectionOutputFormatter.hpp
    MergeSorted.hpp
    NoReferenceFilter.hpp
    RemapContig.hpp
    SnvComparator.hpp
    Sort.hpp
    SortBuffer.hpp
    TypeFilter.hpp
    VariantContig.hpp
    VariantContig.cpp
)

add_library(processors ${SOURCES})
target_link_libraries(processors common fileformats ${Boost_LIBRARIES} ${BZIP2_LIBRARIES} ${ZLIB_LIBRARIES})
