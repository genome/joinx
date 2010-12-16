#include "bedutil/BedStream.hpp"
#include "bedutil/SortBed.hpp"

#include <boost/program_options.hpp>

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>

using namespace std;
namespace po = boost::program_options;

void usage(const char* prog) {
    string progname(prog);
    string::size_type lastSlash = progname.find_last_of("/");
    if (lastSlash != string::npos)
        progname = progname.substr(lastSlash+1);
    cerr << progname << endl;
}

int main(int argc, char** argv) {
    po::options_description opts("Available Options");
    opts.add_options()
        ("help", "this message")
        ("input-file", "input file")
        ("output-file", "output file");
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

    if (vm.count("help")) {
        cerr << opts << endl;
        return 1;
    }

    if (!vm.count("input-file")) {
        cerr << "no input-file specified!" << endl;
        cerr << opts << endl;
        return 1;
    }

    if (!vm.count("output-file")) {
        cerr << "no output-file specified!" << endl;
        cerr << opts << endl;
        return 1;
    }

    string fileIn = vm["input-file"].as<string>();
    string fileOut = vm["output-file"].as<string>();

    ifstream in(fileIn.c_str());
    if (!in) throw runtime_error("Failed to open input file " + fileIn);
    ofstream out(fileOut.c_str());
    if (!out) throw runtime_error("Failed to open ouput file " + fileOut);
    BedStream inp(fileIn, in);
    SortBed sorter(inp, out, 1000000);
    sorter.exec();

    return 0;
}
