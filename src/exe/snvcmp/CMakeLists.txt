cmake_minimum_required(VERSION 2.6)

project(snvcmp)

set(SOURCES main.cpp)

add_executable(snvcmp ${SOURCES})
target_link_libraries(snvcmp bedutil ${Boost_LIBRARIES})

install(TARGETS snvcmp DESTINATION lib/snvcmp-${PROJ_VERSION})
