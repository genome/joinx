#include "fileformats/ReferenceSequence.hpp"
#include "common/Sequence.hpp"

#include <boost/filesystem.hpp>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <gtest/gtest.h>

using namespace std;

namespace {
    struct _foo {
        string chrom;
        string data;
    } testData[] = {
        {"1", "A"},
        {"2", "AC"},
        {"3", "ACG"},
        {"22", "ACGTACGT"},
        {"X", "AA"},
        {"Y", "CC"},
        {"MT", "GG"}
    };
}

class TestReferenceSequence : public ::testing::Test {
protected:
    virtual void SetUp() {
        _tmpdir = "TestRefSeq-tmp-XXXXXX";
        _tmpdir = mkdtemp(&_tmpdir[0]);
        if (_tmpdir.empty()) {
            FAIL() << "failed to make temp directory";
        }

        unsigned fileCount = sizeof(testData)/sizeof(testData[0]);
        for (unsigned i = 0; i < fileCount; ++i) {
            stringstream path;
            path << _tmpdir << "/" << testData[i].chrom << ".bases";
            ofstream f(path.str().c_str());
            if (!f.is_open())
                FAIL() << "Failed to open output file: " << path.str();
            f.write(testData[i].data.c_str(), testData[i].data.size());
        }
    }

    virtual void TearDown() {
        boost::filesystem::remove_all(_tmpdir);
    }

protected:
    std::string _tmpdir;
};

TEST_F(TestReferenceSequence, lookup) {
    ReferenceSequence seq(_tmpdir);
    ASSERT_EQ("A", seq.lookup("1", 1, 1).data());
    ASSERT_EQ("C", seq.lookup("2", 2, 2).data());
    ASSERT_EQ("AC", seq.lookup("2", 1, 2).data());
    ASSERT_EQ("A", seq.lookup("1", 1, 1).data());
    ASSERT_EQ("CG", seq.lookup("3", 2, 3).data());
    ASSERT_EQ("GTA", seq.lookup("22", 3, 5).data());
    ASSERT_EQ("AA", seq.lookup("X", 1, 2).data());
    ASSERT_EQ("CC", seq.lookup("Y", 1, 2).data());
    ASSERT_EQ("GG", seq.lookup("MT", 1, 2).data());
    ASSERT_THROW(seq.lookup("BAD", 1, 2), runtime_error);
    ASSERT_THROW(seq.lookup("1", 11, 12), runtime_error);
}
