#include "fileformats/BedStream.hpp"
#include "bedutil/TypeFilter.hpp"
#include "fileformats/Bed.hpp"
#include "fileformats/InputStream.hpp"

#include <sstream>
#include <stdexcept>
#include <string>
#include <gtest/gtest.h>

using namespace std;

// TODO: use a text fixture for this and hold expected Bed objects to test for
// .extraFields()[1]ity for 1 ASSERT_EQ instead of 6.

const string BEDZ = 
    "# I am a bed file\n"
    "1\t3\t5\tA/T\t43\n"
    "# and I have some comments\n"
    "1\t2\t3\tA/T\t44\n";

TEST(BedStream, next) {
    stringstream data(BEDZ);
    InputStream in("test", data);
    BedStream ss(in, 2);
    ASSERT_FALSE(ss.eof());
    
    Bed bed;
    ASSERT_TRUE(ss >> bed);
    ASSERT_EQ("1", bed.chrom());
    ASSERT_EQ(3u, bed.start());
    ASSERT_EQ(5u, bed.stop());
    ASSERT_EQ("A/T", bed.extraFields()[0]);
    ASSERT_EQ("43", bed.extraFields()[1]);
    ASSERT_EQ(Bed::INDEL, bed.type());
    ASSERT_FALSE(ss.eof());

    ASSERT_TRUE(ss >> bed);
    ASSERT_EQ("1", bed.chrom());
    ASSERT_EQ(2u, bed.start());
    ASSERT_EQ(3u, bed.stop());
    ASSERT_EQ("A/T", bed.extraFields()[0]);
    ASSERT_EQ("44", bed.extraFields()[1]);
    ASSERT_EQ(Bed::SNV, bed.type());

    ASSERT_FALSE(ss.eof());
    ASSERT_FALSE(ss >> bed);
    ASSERT_TRUE(ss.eof());
}

TEST(BedStream, TypeFilterSnv) {
    stringstream data(BEDZ);
    InputStream in("test", data);
        
    BedStream ss(in, 2);
    TypeFilter f(Bed::SNV);
    ss.addFilter(&f);
    ASSERT_FALSE(ss.eof());

    Bed bed;
    ASSERT_TRUE(ss >> bed);
    ASSERT_EQ("1", bed.chrom());
    ASSERT_EQ(2u, bed.start());
    ASSERT_EQ(3u, bed.stop());
    ASSERT_EQ("A/T", bed.extraFields()[0]);
    ASSERT_EQ("44", bed.extraFields()[1]);
    ASSERT_EQ(Bed::SNV, bed.type());

    ASSERT_FALSE(ss.eof());
    ASSERT_FALSE(ss >> bed);
    ASSERT_TRUE(ss.eof());
}

TEST(BedStream, TypeFilterIndel) {
    stringstream data(BEDZ);
    InputStream in("test", data);
        
    BedStream ss(in, 2);
    TypeFilter f(Bed::INDEL);
    ss.addFilter(&f);
    ASSERT_FALSE(ss.eof());

    Bed bed;
    ASSERT_TRUE(ss >> bed);
    ASSERT_EQ("1", bed.chrom());
    ASSERT_EQ(3u, bed.start());
    ASSERT_EQ(5u, bed.stop());
    ASSERT_EQ("A/T", bed.extraFields()[0]);
    ASSERT_EQ("43", bed.extraFields()[1]);
    ASSERT_EQ(Bed::INDEL, bed.type());

    ASSERT_FALSE(ss.eof());
    ASSERT_FALSE(ss >> bed);
    ASSERT_TRUE(ss.eof());
}

TEST(BedStream, peek) {
    stringstream data(BEDZ);
    InputStream in("test", data);

    BedStream ss(in, 2); 
    ASSERT_FALSE(ss.eof());
    
    Bed* peek;
    Bed bed;
    for (unsigned i = 0; i < 2; ++i) {
        ASSERT_TRUE(ss.peek(&peek));
        bed = *peek;
        ASSERT_EQ("1", bed.chrom());
        ASSERT_EQ(3u, bed.start());
        ASSERT_EQ(5u, bed.stop());
        ASSERT_EQ("A/T", bed.extraFields()[0]);
        ASSERT_EQ("43", bed.extraFields()[1]);
        ASSERT_EQ(Bed::INDEL, bed.type());
        ASSERT_FALSE(ss.eof());
    }

    ASSERT_TRUE(ss >> bed);
    ASSERT_EQ("1", bed.chrom());
    ASSERT_EQ(3u, bed.start());
    ASSERT_EQ(5u, bed.stop());
    ASSERT_EQ("A/T", bed.extraFields()[0]);
    ASSERT_EQ("43", bed.extraFields()[1]);
    ASSERT_EQ(Bed::INDEL, bed.type());
    ASSERT_FALSE(ss.eof());

    ASSERT_TRUE(ss.peek(&peek));
    bed = *peek;
    ASSERT_EQ("1", bed.chrom());
    ASSERT_EQ(2u, bed.start());
    ASSERT_EQ(3u, bed.stop());
    ASSERT_EQ("A/T", bed.extraFields()[0]);
    ASSERT_EQ("44", bed.extraFields()[1]);
    ASSERT_EQ(Bed::SNV, bed.type());

    ASSERT_TRUE(ss.next(bed));
    ASSERT_EQ("44", bed.extraFields()[1]);

    ASSERT_FALSE(ss.eof());
    // make sure we can peek at EOF multiple times, and then read once more
    // before getting an exception
    ASSERT_FALSE(ss.peek(&peek));
    ASSERT_FALSE(ss.peek(&peek));
    ASSERT_FALSE(ss.next(bed));
    ASSERT_TRUE(ss.eof());

    // only now, after attempting to read a bed and getting EOF should
    // an exception be thrown
    ASSERT_THROW(ss.peek(&peek), runtime_error);
    ASSERT_THROW(ss >> bed, runtime_error);
}


