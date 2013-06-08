#include "fileformats/vcf/CustomValue.hpp"

#include <cstdint>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <gtest/gtest.h>

using namespace std;
using namespace Vcf;


TEST(VcfCustomValue, scalarInt) {
    CustomType type("DP", CustomType::FIXED_SIZE, 1, CustomType::INTEGER, "depth");
    CustomValue value(&type);

    // set the first element
    value.set<int64_t>(0, 42);
    ASSERT_EQ(1u, value.size());
    ASSERT_FALSE(value.empty());

    const int64_t* result = 0;
    ASSERT_TRUE((result = value.get<int64_t>(0)));
    ASSERT_EQ(42, *result);

    // make sure asking for the wrong type is an error
    ASSERT_THROW(value.get<double>(0), runtime_error);
    ASSERT_THROW(value.get<string>(0), runtime_error);
    ASSERT_THROW(value.get<char>(0), runtime_error);

    // make sure trying to set past the max # of elts is an error
    ASSERT_THROW(value.set<int64_t>(2, 7), runtime_error);

    // make sure trying to get past the max # of elts returns null
    ASSERT_THROW(value.get<int64_t>(2), runtime_error);

    // make sure trying to set with the wrong type is an error
    ASSERT_THROW(value.set<double>(0, 1.2), runtime_error);
    ASSERT_THROW(value.set<string>(0, "hi"), runtime_error);
    ASSERT_THROW(value.set<char>(0, 'a'), runtime_error);
}

TEST(VcfCustomValue, variableSizedListFloat) {
    CustomType type("BQ", CustomType::VARIABLE_SIZE, 0, CustomType::FLOAT, "quality");
    CustomValue value(&type);

    // set the first element
    value.set<double>(0, 0.123);
    value.set<double>(1, 0.456);

    const double* result = 0;
    ASSERT_TRUE((result = value.get<double>(0)));
    ASSERT_EQ(0.123, *result);
    ASSERT_TRUE((result = value.get<double>(1)));
    ASSERT_EQ(0.456, *result);

    // make sure asking for the wrong type is an error
    ASSERT_THROW(value.get<int64_t>(0), runtime_error);
    ASSERT_THROW(value.get<string>(0), runtime_error);
    ASSERT_THROW(value.get<char>(0), runtime_error);

    // make sure set at an arbitrary index works, and updates size (var length)
    ASSERT_NO_THROW(value.set<double>(999, 0.789));
    ASSERT_EQ(1000u, value.size());
    ASSERT_TRUE((result = value.get<double>(999)));
    ASSERT_EQ(0.789, *result);

    // we just extended the size of the array to 1000, yet we have only defined
    // values at indices 0, 1, and 999
    // make sure trying to get an unset value returns null
    ASSERT_FALSE(value.get<double>(2));
    ASSERT_FALSE(value.get<double>(998));

    // make sure trying to set with the wrong type is an error
    ASSERT_THROW(value.set<int64_t>(0, 2), runtime_error);
    ASSERT_THROW(value.set<string>(0, "hi"), runtime_error);
    ASSERT_THROW(value.set<char>(0, 'a'), runtime_error);
}

TEST(VcfCustomValue, fixedSizedListString) {
    CustomType type("CF", CustomType::FIXED_SIZE, 2, CustomType::STRING, "cat food");
    CustomValue value(&type);

    // set the first element
    value.set<string>(0, "tasty");
    value.set<string>(1, "burritos");

    const string* result = 0;
    ASSERT_TRUE((result = value.get<string>(0)));
    ASSERT_EQ("tasty", *result);
    ASSERT_TRUE((result = value.get<string>(1)));
    ASSERT_EQ("burritos", *result);

    // make sure asking for the wrong type is an error
    ASSERT_THROW(value.get<int64_t>(0), runtime_error);
    ASSERT_THROW(value.get<double>(0), runtime_error);
    ASSERT_THROW(value.get<char>(0), runtime_error);

    // make sure trying to set past the max # of elts is an error
    ASSERT_THROW(value.set<string>(2, "boom"), runtime_error);

    // make sure trying to get past the max # of elts returns null
    ASSERT_THROW(value.get<string>(2), runtime_error);

    // make sure trying to set with the wrong type is an error
    ASSERT_THROW(value.set<int64_t>(0, 2), runtime_error);
    ASSERT_THROW(value.set<double>(0, 1.23), runtime_error);
    ASSERT_THROW(value.set<char>(0, 'a'), runtime_error);
}


TEST(VcfCustomValue, fixedIntFromString) {
    CustomType fixedInt("DP", CustomType::FIXED_SIZE, 1, CustomType::INTEGER, "depth");
    ASSERT_NO_THROW(CustomValue(&fixedInt, "12"));
    CustomValue value(&fixedInt, "12");
    const int64_t* resultInt = 0;
    ASSERT_TRUE((resultInt = value.get<int64_t>(0)));
    ASSERT_EQ(12, *resultInt);
    ASSERT_THROW(CustomValue(&fixedInt, "1,2"), runtime_error);
    ASSERT_THROW(CustomValue(&fixedInt, "1.2"), runtime_error);
    ASSERT_THROW(CustomValue(&fixedInt, "pig"), runtime_error);
    ASSERT_THROW(CustomValue(&fixedInt, "c"), runtime_error);
    ASSERT_NO_THROW(CustomValue(&fixedInt, ""));
    ASSERT_NO_THROW(CustomValue(&fixedInt, "."));
    CustomValue empty(&fixedInt, ".");
    ASSERT_TRUE(empty.empty());
    CustomValue empty2(&fixedInt, "");
    ASSERT_TRUE(empty2.empty());
}

TEST(VcfCustomValue, varFloat) {
    CustomType varFloat("BQ", CustomType::VARIABLE_SIZE, 0, CustomType::FLOAT, "quality");
    ASSERT_NO_THROW(CustomValue(&varFloat, "1.2,3.4,5"));
    CustomValue value(&varFloat, "1.2,3.4,5");
    const double* resultInt = 0;
    ASSERT_TRUE((resultInt = value.get<double>(0))); ASSERT_EQ(1.2, *resultInt);
    ASSERT_TRUE((resultInt = value.get<double>(1))); ASSERT_EQ(3.4, *resultInt);
    ASSERT_TRUE((resultInt = value.get<double>(2))); ASSERT_EQ(5, *resultInt);
    ASSERT_THROW(CustomValue(&varFloat, "pig"), runtime_error);
    ASSERT_THROW(CustomValue(&varFloat, "c"), runtime_error);
    ASSERT_NO_THROW(CustomValue(&varFloat, ""));
    ASSERT_NO_THROW(CustomValue(&varFloat, "."));
    CustomValue empty(&varFloat, ".");
    ASSERT_TRUE(empty.empty());
    CustomValue empty2(&varFloat, "");
    ASSERT_TRUE(empty2.empty());
}

TEST(VcfCustomValue, fixedStringFromString) {
    CustomType fixedString("CF", CustomType::FIXED_SIZE, 2, CustomType::STRING, "cat food");
    ASSERT_NO_THROW(CustomValue(&fixedString, "cat,hat"));
    CustomValue value(&fixedString, "cat,hat");
    const string* resultStr;
    ASSERT_TRUE((resultStr = value.get<string>(0))); ASSERT_EQ("cat", *resultStr);
    ASSERT_TRUE((resultStr = value.get<string>(1))); ASSERT_EQ("hat", *resultStr);
    ASSERT_THROW(CustomValue(&fixedString, "1,2,3"), runtime_error);
    ASSERT_NO_THROW(CustomValue(&fixedString, "."));
    CustomValue empty(&fixedString, ".");
    ASSERT_TRUE(empty.empty());
    CustomValue empty2(&fixedString, "");
    ASSERT_TRUE(empty2.empty());

}

TEST(VcfCustomValue, fixedCharFromString) {
    CustomType fixedChar("CH", CustomType::FIXED_SIZE, 3, CustomType::CHAR, "chars?");
    ASSERT_NO_THROW(CustomValue(&fixedChar, "a,b,c"));
    CustomValue value(&fixedChar, "a,b,c");
    const char* resultChar;
    ASSERT_TRUE((resultChar = value.get<char>(0))); ASSERT_EQ('a', *resultChar);
    ASSERT_TRUE((resultChar = value.get<char>(1))); ASSERT_EQ('b', *resultChar);
    ASSERT_TRUE((resultChar = value.get<char>(2))); ASSERT_EQ('c', *resultChar);
    ASSERT_THROW(CustomValue(&fixedChar, "aa"), runtime_error);
    ASSERT_THROW(CustomValue(&fixedChar, "a,bb"), runtime_error);
    ASSERT_THROW(CustomValue(&fixedChar, "a,b,cc"), runtime_error);
    ASSERT_THROW(CustomValue(&fixedChar, "1,2,3,4"), runtime_error);
    ASSERT_NO_THROW(CustomValue(&fixedChar, "."));
    CustomValue empty(&fixedChar, ".");
    ASSERT_TRUE(empty.empty());
    CustomValue empty2(&fixedChar, "");
    ASSERT_TRUE(empty2.empty());

}
//
// CustomType alleleFlag("AF", CustomType::PER_ALLELE, 0, CustomType::FLAG, "allele filtered?");

