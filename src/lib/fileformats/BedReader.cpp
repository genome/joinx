#include "BedReader.hpp"

#include <boost/bind.hpp>

BedReader::ptr openBed(InputStream& in, int maxExtraFields /* = -1*/) {
    BedExtractor ex(boost::bind(&Bed::parseLine, _1, _2, _3, maxExtraFields));
    return BedReader::ptr(new BedReader(ex, in));
}
