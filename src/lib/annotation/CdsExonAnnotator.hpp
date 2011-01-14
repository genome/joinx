#pragma once

#include "TranscriptAnnotator.hpp"

#include <string>

class CdsExonAnnotator : public TranscriptAnnotator {
public:
    std::string codingRegionString(const Variant& v, const TranscriptStructure& structure) const;
};
