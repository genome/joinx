cmake_minimum_required(VERSION 2.6)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

set(LIB_SOURCES
    Bed.cpp
    Bed.hpp
    IResultCollector.hpp
    LineUtils.hpp
    NoReferenceFilter.hpp
    ConcordanceQuality.hpp
    SnpBedReader.hpp
    SnpFilterBase.hpp
    SnpIntersector.cpp
    SnpIntersector.hpp
    SnpStream.cpp
    SnpStream.hpp
    intconfig.hpp
)

add_library(bed ${LIB_SOURCES})
add_executable(snpcmp main.cpp)
target_link_libraries(snpcmp bed)

install(TARGETS snpcmp DESTINATION bin)
