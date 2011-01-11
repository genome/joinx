cmake_minimum_required(VERSION 2.6)

project(cmpbed)

set(SOURCES 
    CmpBedApp.cpp
    CmpBedApp.hpp
    main.cpp
)

set(EXECUTABLE_NAME cmpbed${VERSION_MAJOR}.${VERSION_MINOR})
add_executable(${EXECUTABLE_NAME} ${SOURCES})
target_link_libraries(${EXECUTABLE_NAME} bedutil ${Boost_LIBRARIES})

install(TARGETS ${EXECUTABLE_NAME} DESTINATION bin/)
