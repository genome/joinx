cmake_minimum_required(VERSION 2.8)

project(contrib)

add_executable(mkcontigs MakeContigs.cpp)
target_link_libraries(mkcontigs
    processors reports metrics fileformats io common
    ${Boost_LIBRARIES} ${ZLIB_LIBRARIES})



add_executable(vcf-io-benchmark VcfIoBenchmark.cpp)
target_link_libraries(vcf-io-benchmark
    fileformats io common
    ${Boost_LIBRARIES} ${ZLIB_LIBRARIES})


