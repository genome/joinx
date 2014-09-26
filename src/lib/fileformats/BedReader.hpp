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

struct BedOpener {
    int maxExtraFields;

    BedOpener();
    explicit BedOpener(int maxExtraFields);
    BedReader::ptr operator()(InputStream& in);
};

BedReader::ptr openBed(InputStream& in, int maxExtraFields = -1);
