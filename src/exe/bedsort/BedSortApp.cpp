#include "BedSortApp.hpp"

#include "fileformats/BedStream.hpp"
#include "bedutil/SortBed.hpp"

#include <boost/program_options.hpp>
#include <iostream>
#include <sstream>
#include <stdexcept>

using namespace std;
namespace po = boost::program_options;

BedSortApp::BedSortApp(int argc, char** argv)
{
    parseArguments(argc, argv);
}

void BedSortApp::parseArguments(int argc, char** argv) {
    po::options_description opts("Available Options");
    opts.add_options()
        ("help", "this message")
        ("input-file,i", po::value<string>(&_fileIn), "input file")
        ("output-file,o", po::value<string>(&_fileOut), "output file")
        ("max-lines,M",
            po::value<uint32_t>(&_maxLinesInMem)->default_value(7000000),
            "max number of lines to keep in memory");

    po::positional_options_description posOpts;
    posOpts.add("input-file", 1);
    posOpts.add("output-file", 1);

    po::variables_map vm;
    po::store(
        po::command_line_parser(argc, argv)
            .options(opts)
            .positional(posOpts).run(),
        vm
    );
    po::notify(vm);

    if (vm.count("help")) {
        stringstream ss;
        ss << opts;
        throw runtime_error(ss.str());
    }

    if (!vm.count("input-file")) {
        stringstream ss;
        ss << "no input-file specified!" << endl << endl << opts;
        throw runtime_error(ss.str());
    }

    if (!vm.count("output-file")) {
        stringstream ss;
        ss << "no output-file specified!" << endl << endl << opts;
        throw runtime_error(ss.str());
    }
}

void BedSortApp::exec() {
    ifstream in(_fileIn.c_str());
    if (!in) 
        throw runtime_error("Failed to open input file '" + _fileIn + "'");

    ofstream out(_fileOut.c_str());
    if (!out)
        throw runtime_error("Failed to open ouput file '" + _fileOut + "'");

    BedStream inp(_fileIn, in);
    SortBed sorter(inp, out, _maxLinesInMem);
    sorter.exec();
}
