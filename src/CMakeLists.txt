cmake_minimum_required(VERSION 2.6)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

set(LIB_SOURCES
    Bed.cpp
    Bed.hpp
    IResultCollector.hpp
    LineUtils.hpp
    NoReferenceFilter.hpp
    ConcordanceQuality.hpp
    SnvFilterBase.hpp
    SnvIntersector.cpp
    SnvIntersector.hpp
    SnvStream.cpp
    SnvStream.hpp
    intconfig.hpp
)

add_library(bed ${LIB_SOURCES})
add_executable(snvcmp main.cpp)
target_link_libraries(snvcmp bed)

install(TARGETS snvcmp DESTINATION lib/snvcmp-${PROJ_VERSION})
