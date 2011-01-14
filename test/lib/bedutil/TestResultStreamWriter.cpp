#include "bedutil/ResultStreamWriter.hpp"
#include "fileformats/Bed.hpp"

#include <boost/assign/list_of.hpp>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <gtest/gtest.h>

using boost::assign::list_of;
using namespace std;

// TODO: use a text fixture for this and hold expected Bed objects to test for
// equality for 1 ASSERT_EQ instead of 6.

const string BEDZ = 
    "1\t1\t2\tA/T\t43\n"
    "1\t2\t3\tA/T\t44\n"
    "1\t3\t4\tA/T\t44\n"
    "1\t4\t5\tA/T\t44\n";

class TestResultStreamWriter : public ::testing::Test {
protected:

    void SetUp() {
        vector<string> extraFields = list_of("A/T")("1");
        _beds.push_back(Bed("1", 2, 3, extraFields));
        _beds.push_back(Bed("2", 2, 3, extraFields));
        _beds.push_back(Bed("3", 2, 3, extraFields));
        _beds.push_back(Bed("4", 2, 3, extraFields));
        _beds.push_back(Bed("5", 2, 3, extraFields));
    }

    vector<Bed> _beds;
};

TEST_F(TestResultStreamWriter, all) {
    stringstream hitA;
    stringstream hitB;
    stringstream missA;
    stringstream missB;

    ResultStreamWriter writer(&hitA, &hitB, &missA, &missB);
    writer.hitA(_beds[0]);
    writer.hitB(_beds[1]);
    writer.missA(_beds[2]);
    writer.missB(_beds[3]);

    string line;
    getline(hitA, line);
    ASSERT_EQ(_beds[0].toString(), line);

    getline(hitB, line);
    ASSERT_EQ(_beds[1].toString(), line);

    getline(missA, line);
    ASSERT_EQ(_beds[2].toString(), line);

    getline(missB, line);
    ASSERT_EQ(_beds[3].toString(), line);

}
