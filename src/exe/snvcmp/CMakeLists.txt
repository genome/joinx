cmake_minimum_required(VERSION 2.6)

project(snvcmp)

set(SOURCES 
    ConcordanceApp.cpp
    ConcordanceApp.hpp
    main.cpp
)

add_executable(snvcmp ${SOURCES})
target_link_libraries(snvcmp bedutil ${Boost_LIBRARIES})

install(TARGETS snvcmp DESTINATION lib/snvcmp-${PROJ_VERSION})
