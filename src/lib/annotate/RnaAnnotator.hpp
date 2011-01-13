#pragma once

#include "TranscriptAnnotator.hpp"

class RnaAnnotator : public TranscriptAnnotator {
public:
    std::string codingRegionString(const Variant& v, const TranscriptStructure& structure) const;

};
