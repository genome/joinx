#include "io/StreamJoin.hpp"

#include <gtest/gtest.h>

#include <boost/assign/list_of.hpp>

#include <sstream>
#include <string>
#include <vector>

using boost::assign::list_of;

TEST(TestStreamJoin, integers) {
    std::vector<int> empty;
    std::vector<int> ints = list_of(1)(2)(3);

    std::stringstream ssSome;
    ssSome << streamJoin(ints).emptyString("NOTHING").delimiter(" THEN ");
    EXPECT_EQ("1 THEN 2 THEN 3", ssSome.str());

    std::stringstream ssNone;
    ssNone << streamJoin(empty).emptyString("NOTHING").delimiter(" THEN ");
    EXPECT_EQ("NOTHING", ssNone.str());
}
