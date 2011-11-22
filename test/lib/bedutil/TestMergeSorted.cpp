#include "bedutil/MergeSorted.hpp"
#include "fileformats/Bed.hpp"
#include "fileformats/TypedStream.hpp"

#include <gtest/gtest.h>
#include <functional>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;
using namespace std::placeholders;

namespace {
    const int CHROM_MAX = 22;
    const int START_MAX = 5;
    const int END_MAX   = 5;
    typedef function<void(const BedHeader*, string&, Bed&)> Extractor;
    typedef TypedStream<Bed, Extractor> BedReader;
    Extractor extractor = bind(&Bed::parseLine, _1, _2, _3, -1);

    struct Collector {
        void operator()(const Bed& value) {
            beds.push_back(value);
        }

        vector<Bed> beds;
    };
}

class TestMergeSorted : public ::testing::Test {
protected:

    string chromName(int chrom) {
        stringstream chromStr;
        if (chrom == 23)
            chromStr << "X";
        else if (chrom == 24)
            chromStr << "Y";
        else
            chromStr << chrom;
        return chromStr.str();
    }

    void SetUp() {
        for (int chrom = 1; chrom <= CHROM_MAX; ++chrom) {
            for (int start = 1; start <= START_MAX; ++start) {
                for (int end = 1; end <= END_MAX; ++end) {
                    Bed b(chromName(chrom), start, end);
                    _expectedBeds.push_back(b);
                    _expectedStr << b << "\n";
                }
            }
        }
    }

    vector<Bed> _expectedBeds;
    stringstream _expectedStr;
};

TEST_F(TestMergeSorted, execute) {
    const int nStreams = 10;
    stringstream streams[nStreams];
    vector<Bed>::const_iterator iter = _expectedBeds.begin();
    while (iter != _expectedBeds.end()) {
        for (int i = 0; i < nStreams && iter != _expectedBeds.end(); ++i) {
            streams[i] << *iter++ << "\n";
        }
    }

    vector<InputStream::ptr> inputStreams;
    vector<BedReader::ptr> bedStreams;
    for (int i = 0; i < nStreams; ++i) {
        inputStreams.push_back(InputStream::ptr(new InputStream("test", streams[i])));
        bedStreams.push_back(BedReader::ptr(new BedReader(extractor, **inputStreams.rbegin())));
    }

    Collector c;
    MergeSorted<Bed, BedReader::ptr, Collector> merger(bedStreams, c);
    merger.execute();
    ASSERT_EQ(_expectedBeds.size(), c.beds.size());
    for (unsigned i = 0; i < c.beds.size(); ++i)
        ASSERT_EQ(_expectedBeds[i], c.beds[i]);
}

