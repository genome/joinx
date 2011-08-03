cmake_minimum_required(VERSION 2.8)

project(common)

set(SOURCES
    CigarString.cpp
    CigarString.hpp
    Iub.hpp
    ProgramVersion.hpp
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
