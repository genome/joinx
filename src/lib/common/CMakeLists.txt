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
    TempFile.cpp
    TempFile.hpp
    Tokenizer.hpp
    Variant.cpp
    Variant.hpp
    intconfig.hpp
)

add_library(common ${SOURCES})
target_link_libraries(common ${Boost_LIBRARIES} ${ZLIB_LIBRARIES})
