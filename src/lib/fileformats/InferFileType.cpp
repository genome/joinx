#include "InferFileType.hpp"

#include "BedStream.hpp"
#include "vcf/Reader.hpp"

#include <boost/format.hpp>
#include <fstream>

using boost::format;

namespace {
    template<typename ReaderType>
    bool testReader(const std::string& path) {
        ifstream f(path.c_str());
        if (!f.is_open())
            throw runtime_error(str(format("Failed to open input file %1%") %path));

        try {
            ReaderType reader(path, f);
            typename ReaderType::ValueType value;
            return reader.next(value);
        } catch (...) {
            return false;
        }
    }
}

FileType inferFileType(const std::string& path) {
    if (testReader<BedStream>(path))
        return BED;

    if (testReader<Vcf::Reader>(path))
        return VCF;

    return UNKNOWN;
}
