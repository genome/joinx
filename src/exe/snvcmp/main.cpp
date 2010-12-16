#include "bedutil/BedStream.hpp"
#include "bedutil/ConcordanceQuality.hpp"
#include "bedutil/NoReferenceFilter.hpp"
#include "bedutil/SnvIntersector.hpp"
#include "bedutil/TypeFilter.hpp"

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
        ("file-a,a", po::value<string>(), "input file a (required)")
        ("file-b,b", po::value<string>(), "input file b (required)");
    po::positional_options_description posOpts;
    posOpts.add("file-a", 1);
    posOpts.add("file-b", 1);
    po::variables_map vm;
    po::store(
        po::command_line_parser(argc, argv)
            .options(opts)
            .positional(posOpts).run(),
        vm
    );
    po::notify(vm);

    if (vm.count("help")) {
        cerr << opts << endl;
        return 1;
    }

    if (!vm.count("file-a")) {
        cerr << "no input file-a specified!" << endl;
        cerr << opts << endl;
        return 1;
    }

    if (!vm.count("file-b")) {
        cerr << "no input file-b specified!" << endl;
        cerr << opts << endl;
        return 1;
    }

    string fileA = vm["file-a"].as<string>();
    string fileB = vm["file-b"].as<string>();

    NoReferenceFilter nref;
    TypeFilter snvOnly(Bed::SNV);
    ConcordanceQuality qc;

    ifstream inA(fileA.c_str());
    if (!inA) throw runtime_error("Failed to open input file " + fileA);
    ifstream inB(fileB.c_str());
    if (!inB) throw runtime_error("Failed to open input file " + fileB);

    vector<BedFilterBase*> filters;
    filters.push_back(&nref);
    filters.push_back(&snvOnly);
    BedStream fa(fileA, inA, filters);
    BedStream fb(fileB, inB, filters);

    SnvIntersector snvi(fa, fb, qc);
    snvi.exec();
    qc.report(cout); 

    cout << "Total Snvs: " << fa.bedCount() << endl;
    cout << "      Hits: " << qc.hits() << endl;
    cout << "    Misses: " << qc.misses() << endl;

    return 0;
}
