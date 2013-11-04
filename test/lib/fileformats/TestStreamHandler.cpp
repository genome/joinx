#include "fileformats/StreamHandler.hpp"
#include "fileformats/InputStream.hpp"

#include "common/TempFile.hpp"

#include <gtest/gtest.h>

#include <boost/assign/list_of.hpp>
#include <boost/filesystem.hpp>

#include <cstdio>
#include <istream>
#include <ostream>
#include <stdexcept>
#include <string>

using boost::assign::list_of;
namespace bfs = boost::filesystem;

namespace {
    std::vector<std::string> const messages = list_of
        ("Hello, world!\n")
        ("Goodbye, world!\n")
        ;

    struct StdinRedirector {
        StdinRedirector(std::string const& path) {
            freopen(path.c_str(), "r", stdin);
        }

        ~StdinRedirector() {
            fclose(stdin);
        }
    };
}

class TestStreamHandler : public ::testing::Test {
public:

    void SetUp() {
        for (size_t i = 0; i < messages.size(); ++i) {
            files.push_back(TempFile::create(TempFile::CLEANUP));
            files.back()->stream() << messages[i];
            files.back()->stream().close();
        }

        deleted = TempFile::create(TempFile::CLEANUP);
        bfs::remove(deleted->path());
    }

    void TearDown() {
    }

protected:
    std::vector<TempFile::ptr> files;
    TempFile::ptr deleted;
    StreamHandler streams;
};

TEST_F(TestStreamHandler, fileNotFound) {
    EXPECT_THROW(streams.openForReading(deleted->path()), std::runtime_error);
}

TEST_F(TestStreamHandler, openFileForReading) {
    InputStream::ptr in = streams.openForReading(files[0]->path());
    std::string line;
    EXPECT_TRUE(in->getline(line));
    line += "\n"; // restore trailing newline
    EXPECT_EQ(messages[0], line);

    EXPECT_FALSE(in->eof());
    EXPECT_FALSE(in->getline(line));
    EXPECT_EQ("", line);
    EXPECT_TRUE(in->eof());
}

TEST_F(TestStreamHandler, openManyForReading) {
    std::vector<std::string> paths;
    ASSERT_EQ(files.size(), messages.size());

    for (size_t i = 0; i < files.size(); ++i) {
        paths.push_back(files[i]->path());
    }

    auto ins = streams.openForReading(paths);
    EXPECT_EQ(messages.size(), ins.size());
    for (size_t i = 0; i < paths.size(); ++i) {
        std::string line;
        ins[i]->getline(line);
        line += "\n";
        EXPECT_EQ(messages[i], line);
    }
}

TEST_F(TestStreamHandler, readFromStdin) {
    StdinRedirector redir(files[0]->path());

    InputStream::ptr in = streams.openForReading("-");
    std::string line;
    EXPECT_TRUE(in->getline(line));
    line += "\n";
    EXPECT_EQ(messages[0], line);
}
