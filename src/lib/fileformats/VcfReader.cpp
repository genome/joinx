#include "VcfReader.hpp"

#include "common/compat.hpp"

#include <boost/bind.hpp>

VcfReader::ptr openVcf(InputStream& in) {
    VcfExtractor ex(boost::bind(&Vcf::Entry::parseLine, _1, _2, _3));
    return std::make_unique<VcfReader>(ex, in);
}
