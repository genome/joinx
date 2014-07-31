cmake_minimum_required(VERSION 2.8)

project(annotate)

set(SOURCES
    HomopolymerAnnotator.hpp
    HomopolymerAnnotator.cpp
    SimpleVcfAnnotator.hpp
)

add_library(annotate ${SOURCES})
