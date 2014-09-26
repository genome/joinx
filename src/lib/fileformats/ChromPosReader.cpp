#include "ChromPosReader.hpp"

#include "common/compat.hpp"

#include <boost/bind.hpp>

ChromPosReader::ptr openChromPos(InputStream& in) {
    ChromPosExtractor ex(boost::bind(&ChromPos::parseLine, _1, _2, _3));
    return std::make_unique<ChromPosReader>(ex, in);
}
