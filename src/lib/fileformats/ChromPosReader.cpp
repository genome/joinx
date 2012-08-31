#include "ChromPosReader.hpp"

using namespace std::placeholders;

ChromPosReader::ptr openChromPos(InputStream& in) {
    ChromPosExtractor ex(std::bind(&ChromPos::parseLine, _1, _2, _3));
    return ChromPosReader::ptr(new ChromPosReader(ex, in));
}
