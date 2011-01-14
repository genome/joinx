cmake_minimum_required(VERSION 2.6)

project(common)

set(SOURCES
    Region.cpp
    Region.hpp
    Tokenizer.hpp
    intconfig.hpp
)

add_library(common ${SOURCES})
