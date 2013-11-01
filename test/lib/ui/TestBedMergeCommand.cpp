#include "ui/BedMergeCommand.hpp"

#include <gtest/gtest.h>

#include <boost/program_options.hpp>

#include <boost/assign/list_of.hpp>
#include <stdexcept>

using boost::assign::list_of;
namespace po = boost::program_options;

class TestableBedMergeCommand : public BedMergeCommand {
public:
    using BedMergeCommand::_inputFile;
    using BedMergeCommand::_outputFile;
    using BedMergeCommand::_distance;
};

class TestBedMerge : public ::testing::Test {
public:
    TestableBedMergeCommand cmd;
};

TEST_F(TestBedMerge, inputFileRequired) {
    std::vector<std::string> args;
    EXPECT_THROW(cmd.parseCommandLine(args), po::required_option);
}

TEST_F(TestBedMerge, tooManyPositionalArgs) {
    std::vector<std::string> args = list_of("a.bed")("b.bed");
    EXPECT_THROW(cmd.parseCommandLine(args), po::too_many_positional_options_error);
}

TEST_F(TestBedMerge, tooManyInputFiles) {
    std::vector<std::string> args = list_of("-i")("a.bed")("-i")("b.bed");
    EXPECT_THROW(cmd.parseCommandLine(args), po::multiple_occurrences);
}

TEST_F(TestBedMerge, tooManyInputFilesMix1) {
    std::vector<std::string> args = list_of("a.bed")("-i")("b.bed");
    EXPECT_THROW(cmd.parseCommandLine(args), po::multiple_occurrences);
}

TEST_F(TestBedMerge, tooManyInputFilesMix2) {
    std::vector<std::string> args = list_of("-i")("a.bed")("b.bed");
    EXPECT_THROW(cmd.parseCommandLine(args), po::multiple_occurrences);
}

TEST_F(TestBedMerge, fullCommandLineShort) {
    std::vector<std::string> args = list_of
        ("-i")("in.bed")
        ("-o")("out.bed")
        ("-d")("10")
        ;

    cmd.parseCommandLine(args);
    EXPECT_EQ("in.bed", cmd._inputFile);
    EXPECT_EQ("out.bed", cmd._outputFile);
    EXPECT_EQ(10u, cmd._distance);
}

TEST_F(TestBedMerge, fullCommandLineLong) {
    std::vector<std::string> args = list_of
        ("--input-file")("in.bed")
        ("--output-file")("out.bed")
        ("--distance")("10")
        ;

    cmd.parseCommandLine(args);
    EXPECT_EQ("in.bed", cmd._inputFile);
    EXPECT_EQ("out.bed", cmd._outputFile);
    EXPECT_EQ(10u, cmd._distance);
}

