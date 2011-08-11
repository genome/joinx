cmake_minimum_required(VERSION 2.8)

project(fileformats)

set(SOURCES
    BasesFile.cpp
    BasesFile.hpp
    Bed.cpp
    Bed.hpp
    BedFilterBase.hpp
    BedStream.cpp
    BedStream.hpp
    FastaReader.cpp
    FastaReader.hpp
    IGenomicPosition.hpp
    ISequenceReader.hpp
    ReferenceSequence.cpp
    ReferenceSequence.hpp
    TranscriptStructure.cpp
    TranscriptStructure.hpp
    TranscriptStructureStream.cpp
    TranscriptStructureStream.hpp
    Variant.cpp
    Variant.hpp
    vcf/Entry.cpp
    vcf/Entry.hpp
    vcf/Header.cpp
    vcf/Header.hpp
    vcf/Map.cpp
    vcf/Map.hpp
    vcf/namespace.hpp
    vcf/VariantAdaptor.cpp
    vcf/VariantAdaptor.hpp
    vcf/Validator.cpp
    vcf/Validator.hpp
)

add_library(fileformats ${SOURCES})
target_link_libraries(fileformats common ${Samtools_LIBRARIES})
