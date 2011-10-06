#include "fileformats/vcf/ValueMergers.hpp"
#include "fileformats/vcf/CustomValue.hpp"

#include <cstdint>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <gtest/gtest.h>

using namespace std;
using namespace Vcf;

TEST(TestVcfValueMergers, sum) {
    CustomType type("DP", CustomType::FIXED_SIZE, 1, CustomType::INTEGER, "depth");
}
