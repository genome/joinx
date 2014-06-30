cmake_minimum_required(VERSION 2.8)

project(metrics)

set(SOURCES
    Metrics.cpp
    Metrics.hpp
)

add_library(metrics ${SOURCES})
target_link_libraries(metrics ${Boost_LIBRARIES} ${ZLIB_LIBRARIES})
