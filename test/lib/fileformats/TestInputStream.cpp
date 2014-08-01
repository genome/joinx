#include "io/InputStream.hpp"
#include "io/GZipLineSource.hpp"
#include "common/TempFile.hpp"

#include <fstream>
#include <sstream>
#include <gtest/gtest.h>

using namespace std;

TEST(InputStream, peek) {
    std::stringstream ss("o\nm\ng\n");
    InputStream in("test", ss);
    std::string line;

    EXPECT_EQ('o', in.peek());
    in.getline(line);
    EXPECT_EQ("o", line);

    EXPECT_EQ('m', in.peek());
    in.getline(line);
    EXPECT_EQ("m", line);

    EXPECT_EQ('g', in.peek());
    in.getline(line);
    EXPECT_EQ("g", line);

    EXPECT_EQ(-1, in.peek());
}

TEST(InputStream, caching) {
    stringstream ss("1\n2\n3\n");
    string line;
    InputStream stream("test", ss);
    stream.caching(true);

    ASSERT_TRUE(stream.getline(line));
    ASSERT_EQ("1", line);
    ASSERT_TRUE(stream.getline(line));
    ASSERT_EQ("2", line);
    ASSERT_TRUE(stream.getline(line));
    ASSERT_EQ("3", line);
    ASSERT_FALSE(stream.getline(line));
    ASSERT_TRUE(stream.eof());

    stream.rewind();
    ASSERT_FALSE(stream.eof());
    ASSERT_TRUE(stream.getline(line));
    ASSERT_EQ("1", line);
    ASSERT_TRUE(stream.getline(line));
    ASSERT_EQ("2", line);
    ASSERT_TRUE(stream.getline(line));
    ASSERT_EQ("3", line);
    ASSERT_FALSE(stream.getline(line));
    ASSERT_TRUE(stream.eof());

    stream.caching(false);
    stream.rewind();

    ASSERT_FALSE(stream.eof());
    ASSERT_TRUE(stream.getline(line));
    ASSERT_EQ("1", line);
    ASSERT_TRUE(stream.getline(line));
    ASSERT_EQ("2", line);
    ASSERT_TRUE(stream.getline(line));
    ASSERT_EQ("3", line);
    ASSERT_FALSE(stream.getline(line));
    ASSERT_TRUE(stream.eof());
}

TEST(InputStream, gzipNoNewline) {
    auto tmpFile = TempFile::create(TempFile::CLEANUP);
    ofstream out(tmpFile->path());
    out << "no newline";
    out.close();

    string line;
    GZipLineSource::ptr gzin(new GZipLineSource(tmpFile->path()));
    InputStream in("test", gzin);
    EXPECT_TRUE(in.getline(line));
    EXPECT_EQ("no newline", line);
}
