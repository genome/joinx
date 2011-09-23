#include "InferFileType.hpp"

#include "Bed.hpp"
#include "InputStream.hpp"
#include "TypedStream.hpp"
#include "vcf/Entry.hpp"
#include "vcf/Header.hpp"

#include <boost/format.hpp>
#include <fstream>
#include <functional>

using boost::format;
using namespace std;
using namespace std::placeholders;

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

    template<typename ValueType, typename Extractor>
    bool testReader(InputStream& in, Extractor& extractor) {
        typedef TypedStream<ValueType, Extractor> ReaderType;

        bool rv(false);
        try {
            in.caching(true);
            ReaderType reader(extractor, in);
            typename ReaderType::ValueType value;
            rv = reader.next(value);
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

    typedef function<void(string&, Bed&)> BedExtractor;
    BedExtractor bedExtractor = bind(&Bed::parseLine, _1, _2, -1);
    if (testReader<Bed, BedExtractor>(in, bedExtractor)) {
        rv = BED;
    } else if (testVcf(in)) {
        rv = VCF;
    } else if (testEmpty(in)) {
        rv = EMPTY;
    }

    return rv;
}
