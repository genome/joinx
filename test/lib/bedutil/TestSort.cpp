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
    }

    vector<Bed> _expectedBeds;
    vector<Bed> _shuffledBeds;
    stringstream _expectedStr;
};

TEST_F(TestSort, unstable) {
    const int nStreams = 10;
    stringstream streams[nStreams];
    vector<Bed>::const_iterator iter = _shuffledBeds.begin();
    while (iter != _shuffledBeds.end()) {
        for (int i = 0; i < nStreams && iter != _shuffledBeds.end(); ++i) {
            streams[i] << *iter++ << "\n";
        }
    }

    typedef boost::shared_ptr<BedStream> BedStreamPtr;
    vector<BedStreamPtr> bedStreams;
    for (int i = 0; i < nStreams; ++i)
        bedStreams.push_back(BedStreamPtr(new BedStream("test", streams[i], -1)));

    stringstream out;
    Sort<BedStream, BedStreamPtr> sorter(bedStreams, out, _expectedBeds.size()/10, false);
    sorter.execute();
    ASSERT_EQ(_expectedStr.str(), out.str());
}

TEST_F(TestSort, stable) {
    const int nStreams = 10;
    stringstream streams[nStreams];
    vector<Bed>::const_iterator iter = _shuffledBeds.begin();
    while (iter != _shuffledBeds.end()) {
        for (int i = 0; i < nStreams && iter != _shuffledBeds.end(); ++i) {
            streams[i] << *iter++ << "\n";
        }
    }

    typedef boost::shared_ptr<BedStream> BedStreamPtr;
    vector<BedStreamPtr> bedStreams;
    for (int i = 0; i < nStreams; ++i)
        bedStreams.push_back(BedStreamPtr(new BedStream("test", streams[i], -1)));

    stringstream out;
    Sort<BedStream, BedStreamPtr> sorter(bedStreams, out, _expectedBeds.size()/10, false);
    sorter.execute();
    ASSERT_EQ(_expectedStr.str(), out.str());
}

