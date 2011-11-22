#include "VcfReader.hpp"

using namespace std::placeholders;

VcfReader::ptr openVcf(InputStream& in) {
    VcfExtractor ex(std::bind(&Vcf::Entry::parseLine, _1, _2, _3));
    return VcfReader::ptr(new VcfReader(ex, in));
}
