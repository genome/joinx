cmake_minimum_required(VERSION 2.8)

project(common)

set(SOURCES
    CigarString.cpp
    CigarString.hpp
    Iub.hpp
    JxString.cpp
    JxString.hpp
    MultiType.hpp
    MutationSpectrum.cpp
    MutationSpectrum.hpp
    ProgramDetails.hpp
    Region.cpp
    Region.hpp
    Sequence.cpp
    Sequence.hpp
    TempFile.cpp
    TempFile.hpp
    Timer.hpp
    Tokenizer.hpp
    VariantType.hpp
    namespaces.hpp
)

add_library(common ${SOURCES})
target_link_libraries(common ${Boost_LIBRARIES} ${ZLIB_LIBRARIES})
