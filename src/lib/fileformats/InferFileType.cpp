#include "InferFileType.hpp"

#include "BedStream.hpp"
#include "InputStream.hpp"
#include "vcf/Reader.hpp"

#include <boost/format.hpp>
#include <fstream>

using boost::format;

namespace {
    template<typename ReaderType>
    bool testReader(InputStream& in) {
        bool rv(false);
        try {
            in.caching(true);
            ReaderType reader(in);
            typename ReaderType::ValueType value;
            rv = reader.next(value);
        } catch (...) {
            rv = false;
        }
        in.caching(false);
        in.rewind();
        return rv;
    }
}

FileType inferFileType(InputStream& in) {
    FileType rv(UNKNOWN);

    if (testReader<BedStream>(in))
        rv = BED;
    else if (testReader<Vcf::Reader>(in))
        rv = VCF;

    return rv;
}
