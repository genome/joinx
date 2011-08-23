cmake_minimum_required(VERSION 2.8)

project(fileformats)

set(SOURCES
    BasesFile.cpp
    BasesFile.hpp
    Bed.cpp
    Bed.hpp
    BedFilterBase.hpp
    FastaReader.cpp
    FastaReader.hpp
    IGenomicPosition.hpp
    ISequenceReader.hpp
    InferFileType.cpp
    InferFileType.hpp
    InputStream.cpp
    InputStream.hpp
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
    vcf/Validator.cpp
    vcf/Validator.hpp
    vcf/VariantAdaptor.cpp
    vcf/VariantAdaptor.hpp
    vcf/namespace.hpp
)

add_library(fileformats ${SOURCES})
target_link_libraries(fileformats common ${Samtools_LIBRARIES})
