cmake_minimum_required(VERSION 2.8)

project(reports)

set(SOURCES
    ConcordanceQuality.cpp
    ConcordanceQuality.hpp
    SnvConcordance.cpp
    SnvConcordance.hpp
)

add_library(reports ${SOURCES})
target_link_libraries(reports ${Boost_LIBRARIES} ${ZLIB_LIBRARIES} ${BZIP2_LIBRARIES})
