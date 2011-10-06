#include "fileformats/vcf/MergeActions.hpp"
#include "fileformats/vcf/CustomValue.hpp"

#include <cstdint>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <gtest/gtest.h>

using namespace std;
using namespace Vcf;

TEST(TestVcfMergeActions, sum) {
    CustomType type("DP", CustomType::FIXED_SIZE, 1, CustomType::INTEGER, "depth");
}
