#include "bedutil/MergeSorted.hpp"
#include "fileformats/BedStream.hpp"
#include "fileformats/Bed.hpp"

#include <gtest/gtest.h>
#include <memory>
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

    typedef shared_ptr<BedStream> BedStreamPtr;
    typedef shared_ptr<InputStream> InputStreamPtr;
    vector<InputStreamPtr> inputStreams;
    vector<BedStreamPtr> bedStreams;
    for (int i = 0; i < nStreams; ++i) {
        inputStreams.push_back(InputStreamPtr(new InputStream("test", streams[i])));
        bedStreams.push_back(BedStreamPtr(new BedStream(**inputStreams.rbegin(), -1)));
    }

    stringstream out;
    MergeSorted<Bed, BedStreamPtr> merger(bedStreams, out);
    merger.execute();
    ASSERT_EQ(_expectedStr.str(), out.str());
}

