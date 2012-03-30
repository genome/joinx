cmake_minimum_required(VERSION 2.8)

set(TEST_LIBS common)
include_directories(${GTEST_INCLUDE_DIRS})

def_test(CigarString)
def_test(Iub)
def_test(JxString)
def_test(MutationSpectrum)
def_test(Region)
def_test(Sequence)
def_test(Tokenizer)
