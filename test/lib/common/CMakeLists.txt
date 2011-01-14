cmake_minimum_required(VERSION 2.6)

set(TEST_LIBS common)
include_directories(${GTEST_INCLUDE_DIRS})

def_test(Region)
def_test(Sequence)
def_test(Tokenizer)
