cmake_minimum_required(VERSION 2.8)

project(io)

set(SOURCES
    GZipLineSource.cpp
    GZipLineSource.hpp
    ILineSource.hpp
    InputStream.cpp
    InputStream.hpp
    StreamHandler.cpp
    StreamHandler.hpp
    StreamJoin.hpp
    StreamLineSource.cpp
    StreamLineSource.hpp
    TempFile.cpp
    TempFile.hpp
)

add_library(io ${SOURCES})
target_link_libraries(io ${Boost_LIBRARIES} ${ZLIB_LIBRARIES})
