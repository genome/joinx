#include "ChromPosReader.hpp"

#include <boost/bind.hpp>

ChromPosReader::ptr openChromPos(InputStream& in) {
    ChromPosExtractor ex(boost::bind(&ChromPos::parseLine, _1, _2, _3));
    return ChromPosReader::ptr(new ChromPosReader(ex, in));
}
