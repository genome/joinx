#include "BedReader.hpp"

using namespace std::placeholders;

BedReader::ptr openBed(InputStream& in, int maxExtraFields /* = -1*/) {
    BedExtractor ex(std::bind(&Bed::parseLine, _1, _2, _3, maxExtraFields));
    return BedReader::ptr(new BedReader(ex, in));
}
