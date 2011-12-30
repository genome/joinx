cmake_minimum_required(VERSION 2.8)

project(contrib)

set(SOURCES_EXAMPLE
    example_vcf.cpp
)

set(SOURCES_REPORT
    vcf_report/vcf_report.cpp
    vcf_report/Metrics.hpp
    vcf_report/Metrics.cpp
)

set(REPORT_EXECUTABLE_NAME vcf-report)
add_executable(${REPORT_EXECUTABLE_NAME} ${SOURCES_REPORT})
target_link_libraries(${REPORT_EXECUTABLE_NAME} processors fileformats ${Boost_LIBRARIES})
set_target_properties(${REPORT_EXECUTABLE_NAME} PROPERTIES PACKAGE_OUTPUT_NAME ${REPORT_EXECUTABLE_NAME}${EXE_VERSION_SUFFIX})

set(EXAMPLE_EXECUTABLE_NAME vcf-example)
add_executable(${EXAMPLE_EXECUTABLE_NAME} ${SOURCES_EXAMPLE})
target_link_libraries(${EXAMPLE_EXECUTABLE_NAME} processors fileformats ${Boost_LIBRARIES})
set_target_properties(${EXAMPLE_EXECUTABLE_NAME} PROPERTIES PACKAGE_OUTPUT_NAME ${EXAMPLE_EXECUTABLE_NAME}${EXE_VERSION_SUFFIX})
