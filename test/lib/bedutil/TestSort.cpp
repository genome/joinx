#include "bedutil/Sort.hpp"
#include "fileformats/Bed.hpp"
#include "fileformats/InputStream.hpp"
#include "fileformats/StreamFactory.hpp"

#include <gtest/gtest.h>
#include <algorithm>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <functional>

using namespace std;
using namespace std::placeholders;

namespace {
    const int CHROM_MAX = 22;
    const int START_MAX = 5;
    const int END_MAX   = 5;

    template<typename T>
    struct Collector {
        void operator()(const T& value) {
            out << value << "\n";
        }
        stringstream out;
    };

}

class TestSort : public ::testing::Test {
protected:
    typedef std::function<void(string&, Bed&)> BedExtractor;
    typedef StreamFactory<Bed, BedExtractor> BedReaderFactory;
    typedef Sort<BedReaderFactory, Collector<Bed> > SortType;

    TestSort()
        : _rawStreams(NULL)
        , _bedExtractor(bind(&Bed::parseLine, _1, _2, 0))
        , _streamFactory(_bedExtractor)
    {}

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
            _inputStreams.push_back(InputStream::ptr(new InputStream("test", _rawStreams[i])));
    }

    void TearDown() {
        delete[] _rawStreams;
        _rawStreams = NULL;
    }

    vector<Bed> _expectedBeds;
    vector<Bed> _shuffledBeds;
    stringstream _expectedStr;

    stringstream* _rawStreams;
    vector<InputStream::ptr> _inputStreams;
    BedExtractor _bedExtractor;
    BedReaderFactory _streamFactory;
};

TEST_F(TestSort, unstable) {
    Collector<Bed> out;
    SortType sorter(_streamFactory, _inputStreams, out, _expectedBeds.size()/10, false);
    sorter.execute();
    ASSERT_EQ(_expectedStr.str(), out.out.str());
}

TEST_F(TestSort, zlib) {
    Collector<Bed> out;
    SortType sorter(_streamFactory, _inputStreams, out, _expectedBeds.size()/10, false, ZLIB);
    sorter.execute();
    ASSERT_EQ(_expectedStr.str(), out.out.str());
}

TEST_F(TestSort, bzip2) {
    Collector<Bed> out;
    SortType sorter(_streamFactory, _inputStreams, out, _expectedBeds.size()/10, false, BZIP2);
    sorter.execute();
    ASSERT_EQ(_expectedStr.str(), out.out.str());
}

TEST_F(TestSort, gzip) {
    Collector<Bed> out;
    SortType sorter(_streamFactory, _inputStreams, out, _expectedBeds.size()/10, false, GZIP);
    sorter.execute();
    ASSERT_EQ(_expectedStr.str(), out.out.str());
}

TEST_F(TestSort, stable) {
    Collector<Bed> out;
    SortType sorter(_streamFactory, _inputStreams, out, _expectedBeds.size()/10, true);
    sorter.execute();
    ASSERT_EQ(_expectedStr.str(), out.out.str());
}

