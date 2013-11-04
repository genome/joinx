#include "io/GZipLineSource.hpp"

#include "common/TempFile.hpp"

#include <gtest/gtest.h>

#include <zlib.h>

#include <boost/filesystem.hpp>
#include <boost/assign/list_of.hpp>

#include <cstddef>
#include <cstdlib>
#include <ctime>
#include <sstream>
#include <string>
#include <stdexcept>

using boost::assign::list_of;
namespace bfs = boost::filesystem;

namespace {
    enum {
        TRAILING_NEWLINE = 0,
        NO_TRAILING_NEWLINE = 1
    };

    size_t const numLines = 20;
    size_t const minLineLength = 80;
    std::vector<std::string> words = list_of
        ("signed")
        ("unsigned")
        ("char")
        ("short")
        ("int")
        ("long")
        ("long long")
        ("float")
        ("double")
        ("typedef")
        ("struct")
        ("class")
        ("public")
        ("protected")
        ("private")
        ;

    std::string randomLine(size_t minLength) {
        std::string line;
        while (line.size() < minLength) {
            size_t idx(drand48() * words.size());
            line += words[idx] + " ";
        }
        return line;
    }

    void writeCompressed(std::string const& path, std::string const& data) {
        auto fp = gzopen(path.c_str(), "wb");
        int written(0);
        char const* buf = data.data();
        int remain = data.size();
        while ((written = gzwrite(fp, buf, remain)) > 0) {
            remain -= written;
            buf += written;
        }
        if (remain) {
            throw std::runtime_error("Failed to write compressed temp file!");
        }
        gzclose(fp);

    }
}

class TestGZLineSourceRandom : public ::testing::Test {
public:
    void SetUp() {
        srand48(time(NULL));
        std::stringstream ss;
        for (size_t i = 0; i < numLines; ++i) {
            ss << randomLine(minLineLength) << "\n";
        }
        _data[TRAILING_NEWLINE] = ss.str();
        _data[NO_TRAILING_NEWLINE] = _data[TRAILING_NEWLINE].substr(
            0, _data[TRAILING_NEWLINE].size() - 1);

        for (int i = 0; i < 2; ++i) {
            _tmpFiles[i] = TempFile::create(TempFile::CLEANUP);
            _tmpFiles[i]->stream().close();
            writeCompressed(_tmpFiles[i]->path(), _data[i]);
        }

    }

    std::string _data[2];
    TempFile::ptr _tmpFiles[2];
};

TEST_F(TestGZLineSourceRandom, trailingNewline) {
    GZipLineSource input(_tmpFiles[TRAILING_NEWLINE]->path());
    EXPECT_TRUE(input);
    std::string line;
    std::stringstream result;
    while (input.getline(line)) {
        result << line;
        if (!input.eof()) {
            result << "\n";
        }
    }

    EXPECT_EQ(_data[TRAILING_NEWLINE], result.str());
}

TEST_F(TestGZLineSourceRandom, noTrailingNewline) {
    GZipLineSource input(_tmpFiles[NO_TRAILING_NEWLINE]->path());
    EXPECT_TRUE(input);
    std::string line;
    std::stringstream result;
    while (input.getline(line)) {
        result << line;
        if (!input.eof()) {
            result << "\n";
        }
    }

    EXPECT_EQ(_data[NO_TRAILING_NEWLINE], result.str());
}

TEST(TestGZLineSource, invalidPath) {
    TempFile::ptr tmp = TempFile::create(TempFile::CLEANUP);
    bfs::remove(tmp->path());
    GZipLineSource input(tmp->path());
    EXPECT_FALSE(input);
}
