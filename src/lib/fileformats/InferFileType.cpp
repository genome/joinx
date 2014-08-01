#include "InferFileType.hpp"

#include "Bed.hpp"
#include "ChromPos.hpp"
#include "io/InputStream.hpp"
#include "TypedStream.hpp"
#include "vcf/Entry.hpp"
#include "vcf/Header.hpp"

#include <boost/bind.hpp>
#include <boost/format.hpp>
#include <boost/function.hpp>

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

    typedef boost::function<void(const BedHeader*, string&, Bed&)> BedExtractor;
    BedExtractor bedExtractor = boost::bind(&Bed::parseLine, _1, _2, _3, -1);

    typedef boost::function<void(const ChromPosHeader*, string&, ChromPos&)> ChromPosExtractor;
    ChromPosExtractor cpExtractor = boost::bind(&ChromPos::parseLine, _1, _2, _3);

    if (testReader<Bed, BedExtractor>(in, bedExtractor)) {
        rv = BED;
    } else if (testVcf(in)) {
        rv = VCF;
    } else if (testReader<ChromPos, ChromPosExtractor>(in, cpExtractor)) {
        rv = CHROMPOS;
    } else if (testEmpty(in)) {
        rv = EMPTY;
    }

    return rv;
}
