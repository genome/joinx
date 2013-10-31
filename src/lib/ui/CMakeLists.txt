cmake_minimum_required(VERSION 2.8)

project(ui)

set(SOURCES
    CommandBase.cpp
    CommandBase.hpp
)


add_library(ui ${SOURCES})
target_link_libraries(ui
    common fileformats
    ${Boost_LIBRARIES} ${ZLIB_LIBRARIES} ${BZIP2_LIBRARIES}
)
