cmake_minimum_required(VERSION 2.8)

project(joinx_app)

set(SOURCES
    BedMergeCommand.cpp
    BedMergeCommand.hpp
    CheckRefCommand.cpp
    CheckRefCommand.hpp
    CommandBase.cpp
    CommandBase.hpp
    CreateContigsCommand.cpp
    CreateContigsCommand.hpp
    GenerateCommand.cpp
    GenerateCommand.hpp
    IntersectCollector.hpp
    IntersectCommand.cpp
    IntersectCommand.hpp
    JoinX.cpp
    JoinX.hpp
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
    VcfFilterCommand.cpp
    VcfFilterCommand.hpp
    VcfMergeCommand.cpp
    VcfMergeCommand.hpp
    VcfNormalizeIndelsCommand.cpp
    VcfNormalizeIndelsCommand.hpp
    VcfReportCommand.cpp
    VcfReportCommand.hpp
    VcfSiteFilterCommand.cpp
    VcfSiteFilterCommand.hpp
    Wig2BedCommand.cpp
    Wig2BedCommand.hpp
    main.cpp
)

set(EXECUTABLE_NAME joinx)
add_executable(${EXECUTABLE_NAME} ${SOURCES})
target_link_libraries(${EXECUTABLE_NAME}
    processors reports metrics fileformats common
    ${Boost_LIBRARIES} ${ZLIB_LIBRARIES} ${BZIP2_LIBRARIES})
set_target_properties(${EXECUTABLE_NAME} PROPERTIES PACKAGE_OUTPUT_NAME ${EXECUTABLE_NAME}${EXE_VERSION_SUFFIX})
install(TARGETS ${EXECUTABLE_NAME} DESTINATION bin/)
