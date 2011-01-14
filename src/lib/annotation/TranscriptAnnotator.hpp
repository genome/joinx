#pragma once

#include <vector>
#include <string>
#include <map>

class Variant;
class TranscriptStructure;

struct Annotation {

    Annotation()
        : trv_type("NULL")
        , c_position("NULL")
        , amino_acid_length("NULL")
        , amino_acid_change("NULL")
    {}

    std::string trv_type;
    std::string c_position;
    std::string amino_acid_length;
    std::string amino_acid_change;
};


class TranscriptAnnotator {
public:
    typedef std::map<std::string,std::string> PropertyMapType;
    
    virtual ~TranscriptAnnotator() {}

    virtual std::string codingRegionString(const Variant& v, const TranscriptStructure& structure) const = 0;
    virtual PropertyMapType annotate(const Variant& v, const TranscriptStructure& structure) const = 0;
};
