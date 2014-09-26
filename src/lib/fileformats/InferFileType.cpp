#include "InferFileType.hpp"

#include "Bed.hpp"
#include "ChromPos.hpp"
#include "io/InputStream.hpp"
#include "TypedStream.hpp"
#include "vcf/Entry.hpp"
#include "vcf/Header.hpp"

#include <boost/format.hpp>

#include <fstream>

using boost::format;
using namespace std;

namespace {
    bool testEmpty(InputStream& in) {
        bool rv(true);
        try {
            in.caching(true);
            string line;
            while (!in.eof() && in.good() && in.getline(line)) {
                if (!line.empty() && line[0] != '#') {
                    rv = false;
                    break;
                }

            }
        } catch (...) {
            rv = false;
        }

        in.caching(false);
        in.rewind();
        return rv;
    }

    template<typename ValueType>
    bool testReader(InputStream& in) {
        bool rv(false);
        try {
            in.caching(true);
            auto reader = TypedStreamFactory<DefaultParser<ValueType>>{}(in);
            ValueType value;
            rv = reader->next(value);
        } catch (...) {
            rv = false;
        }
        in.caching(false);
        in.rewind();
        return rv;
    }

    bool testVcf(InputStream& in) {
        bool rv(true);
        try {
            in.caching(true);
            Vcf::Header hdr = Vcf::Header::fromStream(in);
            hdr.assertValid();
            rv = !hdr.empty();
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

    if (testReader<Bed>(in)) {
        rv = BED;
    } else if (testVcf(in)) {
        rv = VCF;
    } else if (testReader<ChromPos>(in)) {
        rv = CHROMPOS;
    } else if (testEmpty(in)) {
        rv = EMPTY;
    }

    return rv;
}
