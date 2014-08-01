#include "parse/Kvp.hpp"

#include "io/InputStream.hpp"

#include <gtest/gtest.h>

#include <map>

TEST(TestParse, Kvp) {
    std::stringstream ss(
        "a=b\n"
        "alpha=beta\n"
        "one=1\n"
        );

    std::map<std::string, std::string> xs;
    InputStream in("test", ss);
    parseKvp(in, xs);

    EXPECT_EQ(3u, xs.size());
    EXPECT_EQ("b", xs["a"]);
    EXPECT_EQ("beta", xs["alpha"]);
    EXPECT_EQ("1", xs["one"]);
}
