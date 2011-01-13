#pragma once

#include "TranscriptAnnotator.hpp"

#include <string>

class IntronAnnotator : public TranscriptAnnotator {
public:
    using TranscriptAnnotator::PropertyMapType;

    std::string codingRegionString(const Variant& v, const TranscriptStructure& structure) const;
    PropertyMapType annotate(const Variant& v, const TranscriptStructure& structure) const;
};
