cmake_minimum_required(VERSION 2.6)

project(snvcmp)

set(SOURCES 
    ConcordanceApp.cpp
    ConcordanceApp.hpp
    main.cpp
)

set(EXECUTABLE_NAME snvcmp${VERSION_MAJOR}.${VERSION_MINOR})
add_executable(${EXECUTABLE_NAME} ${SOURCES})
target_link_libraries(${EXECUTABLE_NAME} bedutil ${Boost_LIBRARIES})

install(TARGETS ${EXECUTABLE_NAME} DESTINATION bin/)
