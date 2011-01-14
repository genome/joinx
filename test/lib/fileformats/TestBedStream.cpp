#include "fileformats/BedStream.hpp"
#include "bedutil/TypeFilter.hpp"
#include "fileformats/Bed.hpp"

#include <sstream>
#include <stdexcept>
#include <string>
#include <gtest/gtest.h>

using namespace std;

// TODO: use a text fixture for this and hold expected Bed objects to test for
// equality for 1 ASSERT_EQ instead of 6.

const string BEDZ = 
    "1\t3\t5\tA/T\t43\n"
    "1\t2\t3\tA/T\t44\n";

TEST(BedStream, next) {
    stringstream data(BEDZ);
    BedStream ss("test", data);
    ASSERT_FALSE(ss.eof());
    
    Bed bed;
    ASSERT_TRUE(ss >> bed);
    ASSERT_EQ("1", bed.chrom);
    ASSERT_EQ(3u, bed.start);
    ASSERT_EQ(5u, bed.end);
    ASSERT_EQ("A/T", bed.refCall);
    ASSERT_EQ("43", bed.qual);
    ASSERT_EQ(Bed::INDEL, bed.type());
    ASSERT_FALSE(ss.eof());

    ASSERT_TRUE(ss >> bed);
    ASSERT_EQ("1", bed.chrom);
    ASSERT_EQ(2u, bed.start);
    ASSERT_EQ(3u, bed.end);
    ASSERT_EQ("A/T", bed.refCall);
    ASSERT_EQ("44", bed.qual);
    ASSERT_EQ(Bed::SNV, bed.type());

    ASSERT_FALSE(ss.eof());
    ASSERT_FALSE(ss >> bed);
    ASSERT_TRUE(ss.eof());
}

TEST(BedStream, TypeFilterSnv) {
    stringstream data(BEDZ);
        
    BedStream ss("test", data);
    TypeFilter f(Bed::SNV);
    ss.addFilter(&f);
    ASSERT_FALSE(ss.eof());

    Bed bed;
    ASSERT_TRUE(ss >> bed);
    ASSERT_EQ("1", bed.chrom);
    ASSERT_EQ(2u, bed.start);
    ASSERT_EQ(3u, bed.end);
    ASSERT_EQ("A/T", bed.refCall);
    ASSERT_EQ("44", bed.qual);
    ASSERT_EQ(Bed::SNV, bed.type());

    ASSERT_FALSE(ss.eof());
    ASSERT_FALSE(ss >> bed);
    ASSERT_TRUE(ss.eof());
}

TEST(BedStream, TypeFilterIndel) {
    stringstream data(BEDZ);
        
    BedStream ss("test", data);
    TypeFilter f(Bed::INDEL);
    ss.addFilter(&f);
    ASSERT_FALSE(ss.eof());

    Bed bed;
    ASSERT_TRUE(ss >> bed);
    ASSERT_EQ("1", bed.chrom);
    ASSERT_EQ(3u, bed.start);
    ASSERT_EQ(5u, bed.end);
    ASSERT_EQ("A/T", bed.refCall);
    ASSERT_EQ("43", bed.qual);
    ASSERT_EQ(Bed::INDEL, bed.type());

    ASSERT_FALSE(ss.eof());
    ASSERT_FALSE(ss >> bed);
    ASSERT_TRUE(ss.eof());
}

TEST(BedStream, peek) {
    stringstream data(BEDZ);

    BedStream ss("test", data); 
    ASSERT_FALSE(ss.eof());
    
    Bed* peek;
    Bed bed;
    for (unsigned i = 0; i < 2; ++i) {
        ASSERT_TRUE(ss.peek(&peek));
        bed = *peek;
        ASSERT_EQ("1", bed.chrom);
        ASSERT_EQ(3u, bed.start);
        ASSERT_EQ(5u, bed.end);
        ASSERT_EQ("A/T", bed.refCall);
        ASSERT_EQ("43", bed.qual);
        ASSERT_EQ(Bed::INDEL, bed.type());
        ASSERT_FALSE(ss.eof());
    }

    ASSERT_TRUE(ss >> bed);
    ASSERT_EQ("1", bed.chrom);
    ASSERT_EQ(3u, bed.start);
    ASSERT_EQ(5u, bed.end);
    ASSERT_EQ("A/T", bed.refCall);
    ASSERT_EQ("43", bed.qual);
    ASSERT_EQ(Bed::INDEL, bed.type());
    ASSERT_FALSE(ss.eof());

    ASSERT_TRUE(ss.peek(&peek));
    bed = *peek;
    ASSERT_EQ("1", bed.chrom);
    ASSERT_EQ(2u, bed.start);
    ASSERT_EQ(3u, bed.end);
    ASSERT_EQ("A/T", bed.refCall);
    ASSERT_EQ("44", bed.qual);
    ASSERT_EQ(Bed::SNV, bed.type());

    ASSERT_TRUE(ss >> bed);
    ASSERT_EQ("44", bed.qual);

    ASSERT_FALSE(ss.eof());
    // make sure we can peek at EOF multiple times, and then read once more
    // before getting an exception
    ASSERT_FALSE(ss.peek(&peek));
    ASSERT_FALSE(ss.peek(&peek));
    ASSERT_FALSE(ss >> bed);
    ASSERT_TRUE(ss.eof());

    // only now, after attempting to read (>>) a bed and getting EOF should
    // an exception be thrown
    ASSERT_THROW(ss.peek(&peek), runtime_error);
    ASSERT_THROW(ss >> bed, runtime_error);
}


