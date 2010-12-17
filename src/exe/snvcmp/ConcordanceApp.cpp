#include "ConcordanceApp.hpp"

#include "bedutil/BedStream.hpp"
#include "bedutil/ConcordanceQuality.hpp"
#include "bedutil/NoReferenceFilter.hpp"
#include "bedutil/SnvComparator.hpp"
#include "bedutil/TypeFilter.hpp"

#include <boost/program_options.hpp>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

using namespace std;
namespace po = boost::program_options;

ConcordanceApp::ConcordanceApp(int argc, char** argv)
{
    parseArguments(argc, argv);
}

void ConcordanceApp::parseArguments(int argc, char** argv) {
    po::options_description opts("Available Options");
    opts.add_options()
        ("help", "this message")
        ("file-a,a", po::value<string>(&_fileA), "input file a (required)")
        ("file-b,b", po::value<string>(&_fileB), "input file b (required)");

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
        stringstream ss;
        ss << opts;
        throw runtime_error(ss.str());
    }

    if (!vm.count("file-a")) {
        stringstream ss;
        ss << "no file-a specified!" << endl << endl << opts;
        throw runtime_error(ss.str());
    }

    if (!vm.count("file-b")) {
        stringstream ss;
        ss << "no file-b specified!" << endl << endl << opts;
        throw runtime_error(ss.str());
    }
}

void ConcordanceApp::exec() {

    ifstream inA(_fileA.c_str());
    if (!inA) throw runtime_error("Failed to open input file '" + _fileA + "'");
    ifstream inB(_fileB.c_str());
    if (!inB) throw runtime_error("Failed to open input file '" + _fileB + "'");

    // set up input filters, keep SNV only, and reject entries with N ref value
    NoReferenceFilter nref;
    TypeFilter snvOnly(Bed::SNV);

    BedStream fa(_fileA, inA);
    fa.addFilter(&snvOnly);
    fa.addFilter(&nref);

    BedStream fb(_fileB, inB);
    fb.addFilter(&snvOnly);

    ConcordanceQuality qc;
    SnvComparator snvi(fa, fb, qc);
    snvi.exec();
    qc.report(cout); 

    cout << "Total Snvs: " << fa.bedCount() << endl;
    cout << "      Hits: " << qc.hits() << endl;
    cout << "    Misses: " << qc.misses() << endl;
}
