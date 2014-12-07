cmake_minimum_required(VERSION 2.8)

project(processors)

set(SOURCES
    BedDeduplicator.hpp
    Deref.hpp
    IntersectFull.hpp
    IntersectionOutputFormatter.cpp
    IntersectionOutputFormatter.hpp
    MergeSorted.hpp
    RefStats.cpp
    RefStats.hpp
    RemapContig.hpp
    Sort.hpp
    SortBuffer.hpp
    VariantContig.cpp
    VariantContig.hpp
    VcfEntryMerger.hpp
    VcfFilterer.hpp
    VcfGenotypeMatcher.cpp
    VcfGenotypeMatcher.hpp
    VcfReheaderer.hpp
    VcfSourceIndexDeduplicator.hpp
    VcfToRaw.hpp
    grouping/GroupBySharedRegions.hpp
    grouping/GroupForEach.hpp
    grouping/GroupOverlapping.hpp
    grouping/GroupSorter.hpp
    grouping/GroupSortingWriter.hpp
    grouping/GroupStats.hpp
)

add_library(processors ${SOURCES})
target_link_libraries(processors ${Boost_LIBRARIES} ${ZLIB_LIBRARIES})
