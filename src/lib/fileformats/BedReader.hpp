#pragma once

#include "TypedStream.hpp"
#include "Bed.hpp"

#include <string>

struct BedParser {
    typedef Bed ValueType;

    int maxExtraFields;

    BedParser();
    explicit BedParser(int maxExtraFields);
    void operator()(BedHeader const* h, std::string& line, Bed& bed);
};

typedef TypedStream<BedParser> BedReader;

BedReader::ptr openBed(InputStream& in, int maxExtraFields = -1);
