cmake_minimum_required(VERSION 2.6)

project(bedsort)

set(SOURCES main.cpp)

add_executable(bedsort ${SOURCES})
target_link_libraries(bedsort bedutil ${Boost_LIBRARIES})

install(TARGETS bedsort DESTINATION lib/snvcmp-${PROJ_VERSION})
