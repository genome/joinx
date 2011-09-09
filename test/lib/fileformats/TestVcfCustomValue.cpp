#include "fileformats/vcf/CustomValue.hpp"

#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <gtest/gtest.h>

using namespace std;
using namespace Vcf;


TEST(VcfCustomValue, scalarInt) {
    CustomType type("DP", CustomType::FIXED_SIZE, 1, CustomType::INTEGER, "depth");
    CustomValue value(type);

    // set the first element
    value.set<int>(0, 42);
    ASSERT_EQ(1, value.size());
    ASSERT_FALSE(value.empty());

    const int* result = 0;
    ASSERT_TRUE((result = value.get<int>(0)));
    ASSERT_EQ(42, *result);

    // make sure asking for the wrong type is an error
    ASSERT_THROW(value.get<double>(0), runtime_error);
    ASSERT_THROW(value.get<string>(0), runtime_error);
    ASSERT_THROW(value.get<char>(0), runtime_error);

    // make sure trying to set past the max # of elts is an error
    ASSERT_THROW(value.set<int>(2, 7), runtime_error);

    // make sure trying to get past the max # of elts returns null
    ASSERT_THROW(value.get<int>(2), runtime_error);

    // make sure trying to set with the wrong type is an error
    ASSERT_THROW(value.set<double>(0, 1.2), runtime_error);
    ASSERT_THROW(value.set<string>(0, "hi"), runtime_error);
    ASSERT_THROW(value.set<char>(0, 'a'), runtime_error);
}

TEST(VcfCustomValue, variableSizedListFloat) {
    CustomType type("BQ", CustomType::VARIABLE_SIZE, 0, CustomType::FLOAT, "quality");
    CustomValue value(type);

    // set the first element
    value.set<double>(0, 0.123);
    value.set<double>(1, 0.456);
    
    const double* result = 0;
    ASSERT_TRUE((result = value.get<double>(0)));
    ASSERT_EQ(0.123, *result);
    ASSERT_TRUE((result = value.get<double>(1)));
    ASSERT_EQ(0.456, *result);

    // make sure asking for the wrong type is an error
    ASSERT_THROW(value.get<int>(0), runtime_error);
    ASSERT_THROW(value.get<string>(0), runtime_error);
    ASSERT_THROW(value.get<char>(0), runtime_error);

    // make sure set at an arbitrary index works, and updates size (var length)
    ASSERT_NO_THROW(value.set<double>(999, 0.789));
    ASSERT_EQ(1000, value.size());
    ASSERT_TRUE((result = value.get<double>(999)));
    ASSERT_EQ(0.789, *result);

    // we just extended the size of the array to 1000, yet we have only defined
    // values at indices 0, 1, and 999 
    // make sure trying to get an unset value returns null
    ASSERT_FALSE(value.get<double>(2));
    ASSERT_FALSE(value.get<double>(998));

    // make sure trying to set with the wrong type is an error
    ASSERT_THROW(value.set<int>(0, 2), runtime_error);
    ASSERT_THROW(value.set<string>(0, "hi"), runtime_error);
    ASSERT_THROW(value.set<char>(0, 'a'), runtime_error);
}

TEST(VcfCustomValue, fixedSizedListString) {
    CustomType type("CF", CustomType::FIXED_SIZE, 2, CustomType::STRING, "cat food");
    CustomValue value(type);

    // set the first element
    value.set<string>(0, "tasty");
    value.set<string>(1, "burritos");
    
    const string* result = 0;
    ASSERT_TRUE((result = value.get<string>(0)));
    ASSERT_EQ("tasty", *result);
    ASSERT_TRUE((result = value.get<string>(1)));
    ASSERT_EQ("burritos", *result);

    // make sure asking for the wrong type is an error
    ASSERT_THROW(value.get<int>(0), runtime_error);
    ASSERT_THROW(value.get<double>(0), runtime_error);
    ASSERT_THROW(value.get<char>(0), runtime_error);

    // make sure trying to set past the max # of elts is an error
    ASSERT_THROW(value.set<string>(2, "boom"), runtime_error);

    // make sure trying to get past the max # of elts returns null
    ASSERT_THROW(value.get<string>(2), runtime_error);

    // make sure trying to set with the wrong type is an error
    ASSERT_THROW(value.set<int>(0, 2), runtime_error);
    ASSERT_THROW(value.set<double>(0, 1.23), runtime_error);
    ASSERT_THROW(value.set<char>(0, 'a'), runtime_error);
}
