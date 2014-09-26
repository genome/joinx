#include "BedReader.hpp"

#include "common/compat.hpp"

BedParser::BedParser()
    : maxExtraFields(0)
{}

BedParser::BedParser(int maxExtraFields)
    : maxExtraFields(maxExtraFields)
{}

void BedParser::operator()(BedHeader const* h, std::string& line, Bed& bed) {
    return Bed::parseLine(h, line, bed, maxExtraFields);
}

BedReader::ptr openBed(InputStream& in, int maxExtraFields /* = -1*/) {
    return TypedStreamFactory<BedParser>{maxExtraFields}(in);
}
