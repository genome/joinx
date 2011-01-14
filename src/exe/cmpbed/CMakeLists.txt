cmake_minimum_required(VERSION 2.6)

project(cmpbed)

set(SOURCES 
    CmpBedApp.cpp
    CmpBedApp.hpp
    main.cpp
)

set(EXECUTABLE_NAME cmpbed${EXE_VERSION_SUFFIX})
add_executable(${EXECUTABLE_NAME} ${SOURCES})
target_link_libraries(${EXECUTABLE_NAME} bedutil fileformats ${Boost_LIBRARIES})

install(TARGETS ${EXECUTABLE_NAME} DESTINATION bin/)
