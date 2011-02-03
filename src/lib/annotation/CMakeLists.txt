cmake_minimum_required(VERSION 2.6)

project(annotation)

set(SOURCES
    CdsExonAnnotator.cpp
    CdsExonAnnotator.hpp
    FlankAnnotator.cpp
    FlankAnnotator.hpp
    IntersectAnnotation.cpp
    IntersectAnnotation.hpp
    IntronAnnotator.cpp
    IntronAnnotator.hpp
    RnaAnnotator.cpp
    RnaAnnotator.hpp
    TranscriptAnnotator.cpp
    TranscriptAnnotator.hpp
    UtrExonAnnotator.cpp
    UtrExonAnnotator.hpp
)

add_library(annotation ${SOURCES})
target_link_libraries(annotation common fileformats bedutil)
