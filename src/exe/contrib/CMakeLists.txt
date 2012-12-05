cmake_minimum_required(VERSION 2.8)

project(contrib)

set(SOURCES_EXAMPLE
    example_vcf.cpp
)

set(EXAMPLE_EXECUTABLE_NAME vcf-example)
add_executable(${EXAMPLE_EXECUTABLE_NAME} ${SOURCES_EXAMPLE})
target_link_libraries(${EXAMPLE_EXECUTABLE_NAME} processors fileformats ${Boost_LIBRARIES})
set_target_properties(${EXAMPLE_EXECUTABLE_NAME} PROPERTIES PACKAGE_OUTPUT_NAME ${EXAMPLE_EXECUTABLE_NAME}${EXE_VERSION_SUFFIX})

add_executable(mkcontigs MakeContigs.cpp)
target_link_libraries(mkcontigs fileformats)

add_executable(test-normalize-indels TestNormalizeIndels.cpp)
target_link_libraries(test-normalize-indels fileformats)
