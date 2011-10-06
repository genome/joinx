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
    OutputWriter.hpp
    ReferenceSequence.cpp
    ReferenceSequence.hpp
    StreamFactory.hpp
    StreamHandler.cpp
    StreamHandler.hpp
    TranscriptStructure.cpp
    TranscriptStructure.hpp
    TranscriptStructureStream.cpp
    TranscriptStructureStream.hpp
    TypedStream.hpp
    Variant.cpp
    Variant.hpp
    vcf/Builder.cpp
    vcf/Builder.hpp
    vcf/CustomType.cpp
    vcf/CustomType.hpp
    vcf/CustomValue.cpp
    vcf/CustomValue.hpp
    vcf/Entry.cpp
    vcf/Entry.hpp
    vcf/EntryMerger.cpp
    vcf/EntryMerger.hpp
    vcf/GenotypeFormatter.cpp
    vcf/GenotypeFormatter.hpp
    vcf/Header.cpp
    vcf/Header.hpp
    vcf/Map.cpp
    vcf/Map.hpp
    vcf/ValueMergers.cpp
    vcf/ValueMergers.hpp
    vcf/MergeStrategy.cpp
    vcf/MergeStrategy.hpp
    vcf/VariantAdaptor.cpp
    vcf/VariantAdaptor.hpp
    vcf/namespace.hpp
)

add_library(fileformats ${SOURCES})
target_link_libraries(fileformats common ${Samtools_LIBRARIES} ${Boost_LIBRARIES})
