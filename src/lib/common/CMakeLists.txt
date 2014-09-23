cmake_minimum_required(VERSION 2.8)

project(common)

set(SOURCES
    CigarString.cpp
    CigarString.hpp
    CoordinateView.hpp
    CyclicIterator.hpp
    Exceptions.hpp
    Integer.hpp
    Iub.hpp
    LocusCompare.hpp
    MultiType.hpp
    MutationSpectrum.cpp
    MutationSpectrum.hpp
    ProgramDetails.hpp
    Region.cpp
    Region.hpp
    RelOps.hpp
    Sequence.cpp
    Sequence.hpp
    String.hpp
    StringView.hpp
    TempFile.cpp
    TempFile.hpp
    Timer.hpp
    Tokenizer.hpp
    UnknownSequenceError.hpp
    UnsortedDataError.hpp
    VariantType.hpp
    cstdint.hpp
    namespaces.hpp
)

add_library(common ${SOURCES})
target_link_libraries(common ${Boost_LIBRARIES} ${ZLIB_LIBRARIES})
