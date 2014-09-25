#include "fileformats/vcf/LazyValue.hpp"
#include "fileformats/vcf/Header.hpp"

#include <gtest/gtest.h>

#include <sstream>
#include <utility>
#include <vector>

namespace {
    struct Example {
        Example(Vcf::Header const& hdr, std::string line, int x, double y)
            : hdr(&hdr)
            , line(std::move(line))
            , x(x)
            , y(y)
        {}

        bool operator==(Example const& rhs) const {
            return hdr == rhs.hdr && line == rhs.line && x == rhs.x && y == rhs.y;
        }

        bool operator!=(Example const& rhs) const {
            return !(*this == rhs);
        }

        Vcf::Header const* hdr;
        std::string line;
        int x;
        double y;
    };

    std::ostream& operator<<(std::ostream& os, Example const& ex) {
        os << ex.line << " (parsed)";
        return os;
    }
}

TEST(TestVcfLazyValue, parse) {
    Vcf::Header h;

    Vcf::LazyValue<Example> lazy("hi");
    std::stringstream ss;
    ss << lazy;
    EXPECT_EQ("hi", ss.str());

    auto const& x = lazy.get(h, 42, 123.45);
    EXPECT_EQ(&h, x.hdr);
    EXPECT_EQ("hi", x.line);
    EXPECT_EQ(42, x.x);
    EXPECT_DOUBLE_EQ(123.45, x.y);

    ss.str("");
    ss << lazy;
    EXPECT_EQ("hi (parsed)", ss.str());
}

TEST(TestVcfLazyValue, copy_before_parse) {
    Vcf::Header h;

    Vcf::LazyValue<Example> ex1("hi");
    Vcf::LazyValue<Example> ex2("there");

    ex1 = ex2;
    EXPECT_EQ(ex1.get(h, 1, 2), ex2.get(h, 1, 2));

    // should be copies, not the same object
    EXPECT_NE(&ex1.get(h, 1, 2), &ex2.get(h, 1, 2));
}

TEST(TestVcfLazyValue, copy_after_parse) {
    Vcf::Header h;

    Vcf::LazyValue<Example> ex1("hi");
    Vcf::LazyValue<Example> ex2("there");

    ex1.get(h, 1, 2); ex2.get(h, 1, 2);
    EXPECT_NE(ex1.get(h, 1, 2), ex2.get(h, 1, 2));

    ex1 = ex2;
    EXPECT_EQ(ex1.get(h, 1, 2), ex2.get(h, 1, 2));

    // should be copies, not the same object
    EXPECT_NE(&ex1.get(h, 1, 2), &ex2.get(h, 1, 2));
}

TEST(TestVcfLazyValue, move) {
    Vcf::Header h;

    Vcf::LazyValue<Example> ex1("one");
    Vcf::LazyValue<Example> ex2("two");

    ex1.get(h, 1, 2);
    auto orig_addr2 = &ex2.get(h, 1, 2);

    ex1 = std::move(ex2);
    EXPECT_EQ(orig_addr2, &ex1.get(h, 1, 2));

    Vcf::LazyValue<Example> ex3(std::move(ex1));
    EXPECT_EQ(orig_addr2, &ex3.get(h, 1, 2));
}
