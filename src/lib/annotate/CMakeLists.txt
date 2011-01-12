cmake_minimum_required(VERSION 2.6)

project(annotate)

set(SOURCES
    FlankAnnotator.cpp
    FlankAnnotator.hpp
    IntersectAnnotation.cpp
    IntersectAnnotation.hpp
    Region.cpp
    Region.hpp
    RnaAnnotator.cpp
    RnaAnnotator.hpp
    Sequence.cpp
    Sequence.hpp
    TranscriptAnnotator.cpp
    TranscriptAnnotator.hpp
    TranscriptStructure.cpp
    TranscriptStructure.hpp
    UtrExonAnnotator.cpp
    UtrExonAnnotator.hpp
    Variant.cpp
    Variant.hpp
)

add_library(annotate ${SOURCES})
