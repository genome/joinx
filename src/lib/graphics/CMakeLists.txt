cmake_minimum_required(VERSION 2.8)

project(graphics)

set(SOURCES
    VennDiagram.hpp
    VennDiagram.cpp
)

add_library(graphics ${SOURCES})
target_link_libraries(graphics
    ${Cairomm_LIBRARIES}
    ${Boost_LIBRARIES}
    ${ZLIB_LIBRARIES}
    ${BZIP2_LIBRARIES}
)
