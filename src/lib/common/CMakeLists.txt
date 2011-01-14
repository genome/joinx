cmake_minimum_required(VERSION 2.6)

project(common)

set(SOURCES
    Region.cpp
    Region.hpp
    Sequence.cpp
    Sequence.hpp
    Tokenizer.hpp
    Variant.cpp
    Variant.hpp
    intconfig.hpp
)

add_library(common ${SOURCES})
