cmake_minimum_required(VERSION 2.8)

project(contrib)

set(SOURCES
    example_vcf.cpp
)

set(EXECUTABLE_NAME example-vcf)
add_executable(${EXECUTABLE_NAME} ${SOURCES})
target_link_libraries(${EXECUTABLE_NAME} bedutil fileformats ${Boost_LIBRARIES})
set_target_properties(${EXECUTABLE_NAME} PROPERTIES PACKAGE_OUTPUT_NAME ${EXECUTABLE_NAME}${EXE_VERSION_SUFFIX})
