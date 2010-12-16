#include "bedutil/LineUtils.hpp"

#include <stdexcept>
#include <string>
#include <gtest/gtest.h>

namespace {
    string line = "fld1\tfld2\tfld3\tfld4\tfld5\tfld6\tfld7";
}

using namespace std;


TEST(extractInt, ok) {
    string intstr = "1234";
    string badstr = "GL1234.0";

    uint32_t value;
    ASSERT_TRUE(extractInt(&intstr[0], &intstr[intstr.size()], value));
    ASSERT_EQ(1234u, value);
    ASSERT_FALSE(extractInt(&badstr[0], &badstr[badstr.size()], value));
}

TEST(extractField, sequential) {

    string::size_type start = 0;
    string::size_type end = 0;
    string fld;

    extractField(line, end, 0, start, end);
    ASSERT_EQ(0u, start) << "field 1 start position";
    ASSERT_EQ(4u, end) << "field 1 end position";
    fld = string(&line[start], &line[end]);
    ASSERT_EQ("fld1", fld) << "value of field 1";

    ++end;
    extractField(line, end, 0, start, end);
    ASSERT_EQ(5u, start) << "field 2 start position";
    ASSERT_EQ(9u, end) << "field 2 end position";
    fld = string(&line[start], &line[end]);
    ASSERT_EQ("fld2", fld) << "value of field 2";

    ++end;
    extractField(line, end, 0, start, end);
    ASSERT_EQ(10u, start) << "field 2 start position";
    ASSERT_EQ(14u, end) << "field 2 end position";
    fld = string(&line[start], &line[end]);
    ASSERT_EQ("fld3", fld) << "value of field 3";

    ++end;
    extractField(line, end, 0, start, end);
    ASSERT_EQ(15u, start) << "field 2 start position";
    ASSERT_EQ(19u, end) << "field 2 end position";
    fld = string(&line[start], &line[end]);
    ASSERT_EQ("fld4", fld) << "value of field 4";
}

TEST(extractField, skipFields) {
    string::size_type start = 0;
    string::size_type end = 0;
    string fld;

    extractField(line, end, 1, start, end);
    ASSERT_EQ(5u, start) << "field 2 start position";
    ASSERT_EQ(9u, end) << "field 2 end position";
    fld = string(&line[start], &line[end]);
    ASSERT_EQ("fld2", fld) << "value of field 2";

    ++end;
    extractField(line, end, 1, start, end);
    ASSERT_EQ(15u, start) << "field 2 start position";
    ASSERT_EQ(19u, end) << "field 2 end position";
    fld = string(&line[start], &line[end]);
    ASSERT_EQ("fld4", fld) << "value of field 4";

    ++end;
    // Try to skip fields 5, 6, 7, and read non-existing 8, should throw
    ASSERT_THROW(extractField(line, end, 3, start, end), runtime_error)
        <<  "trying to extract past end of line is an error";

    extractField(line, end, 2, start, end);
    ASSERT_EQ(30u, start) << "field 7 start position";
    ASSERT_EQ(34u, end) << "field 7 end position";
    fld = string(&line[start], &line[end]);
    ASSERT_EQ("fld7", fld) << "value of field 7";

    ++end;
    ASSERT_THROW(extractField(line, end, 0, start, end), runtime_error)
        <<  "trying to extract past end of line is an error";
}

