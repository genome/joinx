cmake_minimum_required(VERSION 2.6)

project(snvcmp)

set(SOURCES 
    ConcordanceApp.cpp
    ConcordanceApp.hpp
    main.cpp
)

set(EXECUTABLE_NAME snvcmp)
add_executable(${EXECUTABLE_NAME} ${SOURCES})
target_link_libraries(${EXECUTABLE_NAME} bedutil fileformats ${Boost_LIBRARIES})
set_target_properties(${EXECUTABLE_NAME} PROPERTIES PACKAGE_OUTPUT_NAME ${EXECUTABLE_NAME}${EXE_VERSION_SUFFIX})
install(TARGETS ${EXECUTABLE_NAME} DESTINATION bin/)
