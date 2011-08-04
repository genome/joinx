#include "bedutil/Sort.hpp"
#include "fileformats/BedStream.hpp"
#include "fileformats/Bed.hpp"

#include <gtest/gtest.h>
#include <algorithm>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

namespace {
    const int CHROM_MAX = 22;
    const int START_MAX = 5;
    const int END_MAX   = 5;
}

class TestSort : public ::testing::Test {
protected:
    typedef boost::shared_ptr<BedStream> BedStreamPtr;

    TestSort() : _rawStreams(NULL) {}

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
        _shuffledBeds = _expectedBeds;
        random_shuffle(_shuffledBeds.begin(), _shuffledBeds.end());

        const int nStreams = 10;
        _rawStreams = new stringstream[nStreams];
        auto iter = _shuffledBeds.begin();
        while (iter != _shuffledBeds.end()) {
            for (int i = 0; i < nStreams && iter != _shuffledBeds.end(); ++i) {
                _rawStreams[i] << *iter++ << "\n";
            }
        }

        for (int i = 0; i < nStreams; ++i)
            _bedStreams.push_back(BedStreamPtr(new BedStream("test", _rawStreams[i], -1)));
    }

    void TearDown() {
        delete[] _rawStreams;
        _rawStreams = NULL;
    }

    vector<Bed> _expectedBeds;
    vector<Bed> _shuffledBeds;
    stringstream _expectedStr;

    stringstream* _rawStreams;
    vector<BedStreamPtr> _bedStreams;
};

TEST_F(TestSort, unstable) {
    stringstream out;
    Sort<BedStream, BedStreamPtr> sorter(_bedStreams, out, _expectedBeds.size()/10, false);
    sorter.execute();
    ASSERT_EQ(_expectedStr.str(), out.str());
}

TEST_F(TestSort, zlib) {
    stringstream out;
    Sort<BedStream, BedStreamPtr> sorter(_bedStreams, out, _expectedBeds.size()/10, false, ZLIB);
    sorter.execute();
    ASSERT_EQ(_expectedStr.str(), out.str());
}

TEST_F(TestSort, bzip2) {
    stringstream out;
    Sort<BedStream, BedStreamPtr> sorter(_bedStreams, out, _expectedBeds.size()/10, false, BZIP2);
    sorter.execute();
    ASSERT_EQ(_expectedStr.str(), out.str());
}

TEST_F(TestSort, gzip) {
    stringstream out;
    Sort<BedStream, BedStreamPtr> sorter(_bedStreams, out, _expectedBeds.size()/10, false, GZIP);
    sorter.execute();
    ASSERT_EQ(_expectedStr.str(), out.str());
}

TEST_F(TestSort, stable) {
    stringstream out;
    Sort<BedStream, BedStreamPtr> sorter(_bedStreams, out, _expectedBeds.size()/10, false);
    sorter.execute();
    ASSERT_EQ(_expectedStr.str(), out.str());
}

