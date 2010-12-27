#include "ConcordanceApp.hpp"

#include "bedutil/BedStream.hpp"
#include "bedutil/ConcordanceQuality.hpp"
#include "bedutil/NoReferenceFilter.hpp"
#include "bedutil/ResultMultiplexer.hpp"
#include "bedutil/ResultStreamWriter.hpp"
#include "bedutil/SnvComparator.hpp"
#include "bedutil/TypeFilter.hpp"

#include <boost/program_options.hpp>
#include <fstream>
#include <iostream>
#include <memory>
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
        ("file-b,b", po::value<string>(&_fileB), "input file b (required)")
        ("output-a", po::value<string>(&_outFileA), "output hits in a to file");

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

    auto_ptr<ResultStreamWriter> rsw;

    ifstream inA(_fileA.c_str());
    if (!inA)
        throw runtime_error("Failed to open input file '" + _fileA + "'");
    ifstream inB(_fileB.c_str());
    if (!inB)
        throw runtime_error("Failed to open input file '" + _fileB + "'");
    ofstream outA;
    if (!_outFileA.empty()) {
        outA.open(_outFileA.c_str(), ios::out|ios::binary);
        if (!outA)
            throw runtime_error("Failed to open output file '" + _outFileA + "'");
        rsw.reset(new ResultStreamWriter(&outA, NULL, NULL, NULL));
    }

    // set up input filters, keep SNV only, and reject entries with N ref value
    NoReferenceFilter nref;
    TypeFilter snvOnly(Bed::SNV);

    BedStream fa(_fileA, inA);
    fa.addFilter(&snvOnly);
    fa.addFilter(&nref);

    BedStream fb(_fileB, inB);
    fb.addFilter(&snvOnly);

    ConcordanceQuality qc;
    ResultMultiplexer rmx;
    rmx.add(&qc);
    if (rsw.get())
        rmx.add(rsw.get());
    SnvComparator snvi(fa, fb, rmx);
    snvi.exec();
    qc.report(cout); 

    cout << "Total Snvs: " << fa.bedCount() << endl;
    cout << "      Hits: " << qc.hits() << endl;
    cout << "    Misses: " << qc.misses() << endl;
}
