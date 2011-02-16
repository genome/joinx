cmake_minimum_required(VERSION 2.8)

set(TEST_LIBS common fileformats)
include_directories(${GTEST_INCLUDE_DIRS})

def_test(Region)
def_test(Sequence)
def_test(Tokenizer)
def_test(Variant)
