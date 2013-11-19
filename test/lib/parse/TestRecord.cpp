#include "parse/Elements.hpp"
#include "common/cstdint.hpp"

#include <gtest/gtest.h>

#include <string>
#include <iostream>

namespace {
    struct Chrom        : BasicField<std::string> {};
    struct RefAllele    : BasicField<std::string> {};
    struct Identifiers  : DelimitedField<std::vector<std::string>, ','> {};
    struct Position     : BasicField<int32_t> {};
    struct AltAlleles   : DelimitedField<std::vector<StringView>, ','> {};
    struct Quality      : BasicField<float> {};
    struct Filter       : DelimitedField<std::vector<std::string>, ','> {};
    struct Info         : DelimitedField<std::vector<std::string>, ';'> {};
    struct FormatFields : DelimitedField<std::vector<std::string>, ':'> {};

    namespace mpl = boost::mpl;
    typedef mpl::vector<
        Column<0, Chrom>,
        Column<1, Position>,
        Column<2, Identifiers>,
        Column<3, RefAllele>,
        Column<4, AltAlleles>,
        Column<5, Quality>,
        Column<6, Filter>,
        Column<7, Info>,
        Column<8, FormatFields>
        > VcfRecordSpec;
}

TEST(Record, parse) {
    Record<VcfRecordSpec, TabDelimited> v;

    std::string vcfTxt(
        "20\t"
        "14370\t"
        "rs6054257,foo\t"
        "G\t"
        "A,T\t"
        "29\t"
        "PASS\t"
        "NS=3;DP=14;AF=0.5;DB;H2\t"
        "GT:GQ:DP:HQ"
        );

    v.parse(vcfTxt);

    std::stringstream output;
    v.toStream(output);
    EXPECT_EQ(vcfTxt, output.str());

    EXPECT_EQ("20", v.get<Chrom>());
    EXPECT_EQ(14370, v.get<Position>());

    auto const& identifiers = v.get<Identifiers>();
    ASSERT_EQ(2u, identifiers.size());
    EXPECT_EQ("rs6054257", identifiers[0]);
    EXPECT_EQ("foo", identifiers[1]);

    EXPECT_EQ("G", v.get<RefAllele>());

    auto const& alts = v.get<AltAlleles>();
    ASSERT_EQ(2u, alts.size());
    EXPECT_EQ("A", alts[0]);
    EXPECT_EQ("T", alts[1]);

    auto const& format = v.get<FormatFields>();
    ASSERT_EQ(4u, format.size());
    EXPECT_EQ("GT", format[0]);
    EXPECT_EQ("GQ", format[1]);
    EXPECT_EQ("DP", format[2]);
    EXPECT_EQ("HQ", format[3]);

}
