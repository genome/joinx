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

BedOpener::BedOpener()
    : maxExtraFields(0)
{
}

BedOpener::BedOpener(int maxExtraFields)
    : maxExtraFields(maxExtraFields)
{
}

BedReader::ptr BedOpener::operator()(InputStream& in) {
    BedParser parser(maxExtraFields);
    return std::make_unique<BedReader>(parser, in);
}

BedReader::ptr openBed(InputStream& in, int maxExtraFields /* = -1*/) {
    return BedOpener{maxExtraFields}(in);
}
