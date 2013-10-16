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
    RefStats.cpp
    RefStats.hpp
    RemapContig.hpp
    SnvComparator.hpp
    Sort.hpp
    SortBuffer.hpp
    TypeFilter.hpp
    VariantContig.cpp
    VariantContig.hpp
    VcfToRaw.hpp
)

add_library(processors ${SOURCES})
target_link_libraries(processors ${Boost_LIBRARIES} ${BZIP2_LIBRARIES} ${ZLIB_LIBRARIES})
