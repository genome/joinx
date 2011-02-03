cmake_minimum_required(VERSION 2.6)

project(fileformats)

set(SOURCES
    BasesFile.cpp
    BasesFile.hpp
    Bed.cpp
    Bed.hpp
    BedFilterBase.hpp
    BedStream.cpp
    BedStream.hpp
    IGenomicPosition.hpp
    ISequenceReader.hpp
    ReferenceSequence.cpp
    ReferenceSequence.hpp
    TranscriptStructure.cpp
    TranscriptStructure.hpp
    TranscriptStructureStream.cpp
    TranscriptStructureStream.hpp
)

add_library(fileformats ${SOURCES})
target_link_libraries(fileformats common)
