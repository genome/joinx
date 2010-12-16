#include "BedStream.hpp"
#include "TypeFilter.hpp"
#include "Bed.hpp"

#include <sstream>
#include <stdexcept>
#include <string>
#include <gtest/gtest.h>

using namespace std;

const string BEDZ = 
    "1\t3\t5\tA/T\t43\n"
    "1\t2\t3\tA/T\t44\n";

TEST(BedStream, next) {
    stringstream data(BEDZ);
    BedStream ss("test", data);
    
    Bed bed;
    ASSERT_TRUE(ss >> bed);
    ASSERT_EQ("1", bed.chrom);
    ASSERT_EQ(3u, bed.start);
    ASSERT_EQ(5u, bed.end);
    ASSERT_EQ("A/T", bed.refCall);
    ASSERT_EQ("43", bed.qual);
    ASSERT_EQ(Bed::INDEL, bed.type());

    ASSERT_TRUE(ss >> bed);
    ASSERT_EQ("1", bed.chrom);
    ASSERT_EQ(2u, bed.start);
    ASSERT_EQ(3u, bed.end);
    ASSERT_EQ("A/T", bed.refCall);
    ASSERT_EQ("44", bed.qual);
    ASSERT_EQ(Bed::SNV, bed.type());

    ASSERT_TRUE(ss.eof());
    ASSERT_THROW(ss >> bed, runtime_error);
}

TEST(BedStream, TypeFilterSnv) {
    stringstream data(
        "1\t3\t5\tA/T\t43\n"
        "1\t2\t3\tA/T\t44\n"
    );
        
    TypeFilter f(Bed::SNV);
    BedStream ss("test", data, &f);

    Bed bed;
    ASSERT_TRUE(ss >> bed);
    ASSERT_EQ("1", bed.chrom);
    ASSERT_EQ(2u, bed.start);
    ASSERT_EQ(3u, bed.end);
    ASSERT_EQ("A/T", bed.refCall);
    ASSERT_EQ("44", bed.qual);
    ASSERT_EQ(Bed::SNV, bed.type());
//    ASSERT_EQ(2u, ss.lineNum());

    ASSERT_TRUE(ss.eof());
    ASSERT_THROW(ss >> bed, runtime_error);
}

TEST(BedStream, TypeFilterIndel) {
    stringstream data(
        "1\t3\t5\tA/T\t43\n"
        "1\t2\t3\tA/T\t44\n"
    );
        
    TypeFilter f(Bed::INDEL);
    BedStream ss("test", data, &f);

    Bed bed;
    ASSERT_TRUE(ss >> bed);
    ASSERT_EQ("1", bed.chrom);
    ASSERT_EQ(3u, bed.start);
    ASSERT_EQ(5u, bed.end);
    ASSERT_EQ("A/T", bed.refCall);
    ASSERT_EQ("43", bed.qual);
    ASSERT_EQ(Bed::INDEL, bed.type());
//    ASSERT_EQ(1u, ss.lineNum());

    ASSERT_TRUE(ss.eof());
    ASSERT_THROW(ss >> bed, runtime_error);
}
