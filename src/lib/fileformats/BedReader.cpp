#include "BedReader.hpp"

#include "common/compat.hpp"

#include <boost/bind.hpp>

BedReader::ptr openBed(InputStream& in, int maxExtraFields /* = -1*/) {
    BedExtractor ex(boost::bind(&Bed::parseLine, _1, _2, _3, maxExtraFields));
    return std::make_unique<BedReader>(ex, in);
}
