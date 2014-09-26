#include "ChromPosReader.hpp"

#include "common/compat.hpp"

ChromPosReader::ptr openChromPos(InputStream& in) {
    return TypedStreamFactory<DefaultParser<ChromPos>>{}(in);
}
