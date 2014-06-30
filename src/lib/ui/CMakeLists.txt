cmake_minimum_required(VERSION 2.8)

project(ui)

set(SOURCES
    BedMergeCommand.cpp
    BedMergeCommand.hpp
    CheckRefCommand.cpp
    CheckRefCommand.hpp
    CommandBase.cpp
    CommandBase.hpp
    CreateContigsCommand.cpp
    CreateContigsCommand.hpp
    FindHomopolymersCommand.cpp
    FindHomopolymersCommand.hpp
    GenerateCommand.cpp
    GenerateCommand.hpp
    IntersectCollector.hpp
    IntersectCommand.cpp
    IntersectCommand.hpp
    RefStatsCommand.cpp
    RefStatsCommand.hpp
    RemapCigarCommand.cpp
    RemapCigarCommand.hpp
    SnvConcordanceByQualityCommand.cpp
    SnvConcordanceByQualityCommand.hpp
    SnvConcordanceCommand.cpp
    SnvConcordanceCommand.hpp
    SortCommand.cpp
    SortCommand.hpp
    Vcf2RawCommand.cpp
    Vcf2RawCommand.hpp
    VcfAnnotateCommand.cpp
    VcfAnnotateCommand.hpp
    VcfCompareGtCommand.cpp
    VcfCompareGtCommand.hpp
    VcfFilterCommand.cpp
    VcfFilterCommand.hpp
    VcfMergeCommand.cpp
    VcfMergeCommand.hpp
    VcfNormalizeIndelsCommand.cpp
    VcfNormalizeIndelsCommand.hpp
    VcfRemoveFilteredGtCommand.cpp
    VcfRemoveFilteredGtCommand.hpp
    VcfReportCommand.cpp
    VcfReportCommand.hpp
    VcfSiteFilterCommand.cpp
    VcfSiteFilterCommand.hpp
    Wig2BedCommand.cpp
    Wig2BedCommand.hpp
)


add_library(ui ${SOURCES})
target_link_libraries(ui
    ${Boost_LIBRARIES} ${ZLIB_LIBRARIES}
)
