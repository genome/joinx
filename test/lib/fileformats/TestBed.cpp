#include "fileformats/Bed.hpp"
#include "common/LocusCompare.hpp"

#include <stdexcept>
#include <string>
#include <gtest/gtest.h>

using namespace std;

namespace {
    BedHeader hdr;
}

class TestableBed : public Bed {
public:
    using Bed::_chrom;
    using Bed::_start;
    using Bed::_stop;
};

TEST(Bed, parse) {
    string snvLine = "1\t2\t3\tA/T\t44";
    TestableBed snv;
    Bed::parseLine(&hdr, snvLine, snv, 2);
    ASSERT_EQ("1", snv.chrom());
    ASSERT_EQ(2u, snv.start());
    ASSERT_EQ(3u, snv.stop());
    ASSERT_EQ("A/T", snv.extraFields()[0]);
    ASSERT_EQ("44", snv.extraFields()[1]);
    ASSERT_EQ(1, snv.length());

    ASSERT_EQ(Bed::SNV, snv.type());

    snv._stop += 1;
    ASSERT_EQ(Bed::INDEL, snv.type());
}

TEST(Bed, length) {
    Bed snv("1", 2, 3);
    Bed del2bp("1", 2, 4);
    Bed ins("1", 2, 2);

    ASSERT_EQ(1, snv.length());
    ASSERT_EQ(2, del2bp.length());
    ASSERT_EQ(0, ins.length());
}

TEST(Bed, swap) {
    string snvLine1 = "1\t2\t3\tA/T\t44\t19";
    string snvLine2 = "2\t3\t4\tA/T\t44\t20";
    Bed a, b, oa, ob;
    Bed::parseLine(&hdr, snvLine1, oa, 3);
    Bed::parseLine(&hdr, snvLine2, ob, 3);

    a = oa;
    b = ob;

    a.swap(b);
    ASSERT_EQ(a, ob);
    ASSERT_EQ(b, oa);

    a.swap(b);
    ASSERT_EQ(a, oa);
    ASSERT_EQ(b, ob);

    a.swap(a);
    ASSERT_EQ(a, oa);
}

TEST(Bed, parseBad) {
    Bed b;
    string baddies[] = {
        "",
        "1",
        "1\t2",
    };
    for (unsigned i = 0; i < sizeof(baddies)/sizeof(baddies[0]); ++i) {
        string tmp = baddies[i]; // string gets swapped
        ASSERT_THROW(Bed::parseLine(&hdr, tmp, b), runtime_error) <<
            "i is " << i << ", input was: '" << baddies[i] << "', got: " << b;
    }
}

TEST(Bed, cmp) {
    TestableBed a;
    string data ("1\t1\t1\tA/T\t44");
    Bed::parseLine(&hdr, data, a, 2);

    TestableBed b = a;
    b._stop++;

    LocusCompare<> cmp;
    ASSERT_EQ(0, cmp(a, a)) << "bed chromosome sort: 1 == 1";
    ASSERT_GT(0, cmp(a, b)) << "bed chromosome sort: 1 < 2";
    ASSERT_LT(0, cmp(b, a));

    b = a;
    b._start++;
    ASSERT_GT(0, cmp(a, b));
    ASSERT_LT(0, cmp(b, a));

    b = a;
    b._chrom = "2";
    ASSERT_GT(0, cmp(a, b));
    ASSERT_LT(0, cmp(b, a));

    a._chrom = "22";
    b = a;
    b._chrom = "X";
    ASSERT_GT(0, cmp(a, b)) << "bed chromosome sort: 22 < X";
    ASSERT_LT(0, cmp(b, a)) << "bed chromosome sort: X > 22";
}

