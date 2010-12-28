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
        ("hits-a",   po::value<string>(&_hitFileA), "output hits in 'a' to this file")
        ("hits-b",   po::value<string>(&_hitFileB), "output hits in 'b' to this file")
        ("miss-a",   po::value<string>(&_missFileA), "output misses in 'a' to this file")
        ("miss-b",   po::value<string>(&_missFileB), "output misses in 'b' to this file");

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

auto_ptr<ResultStreamWriter> ConcordanceApp::setupStreamWriter() {
    auto_ptr<ResultStreamWriter> resultStreamWriter;

    ofstream* hitA(NULL);
    ofstream* hitB(NULL);
    ofstream* missA(NULL);
    ofstream* missB(NULL);

    if (!_hitFileA.empty()) {
        _hitA.open(_hitFileA.c_str(), ios::out|ios::binary);
        if (!_hitA)
            throw runtime_error("Failed to open output file for hits in 'a': '" + _hitFileA + "'");
        hitA = &_hitA;
    }

    if (!_hitFileB.empty()) {
        _hitB.open(_hitFileB.c_str(), ios::out|ios::binary);
        if (!_hitB)
            throw runtime_error("Failed to open output file for hits in 'a': '" + _hitFileB + "'");
        hitB = &_hitB;
    }

    if (!_missFileA.empty()) {
        _missA.open(_missFileA.c_str(), ios::out|ios::binary);
        if (!_missA)
            throw runtime_error("Failed to open output file for misss in 'a': '" + _missFileA + "'");
        missA = &_missA;
    }

    if (!_missFileB.empty()) {
        _missB.open(_missFileB.c_str(), ios::out|ios::binary);
        if (!_missB)
            throw runtime_error("Failed to open output file for misss in 'a': '" + _missFileB + "'");
        missB = &_missB;
    }

    resultStreamWriter.reset(new ResultStreamWriter(hitA, hitB, missA, missB));
    return resultStreamWriter;
}

void ConcordanceApp::exec() {

    ifstream inA(_fileA.c_str());
    if (!inA)
        throw runtime_error("Failed to open input file '" + _fileA + "'");
    ifstream inB(_fileB.c_str());
    if (!inB)
        throw runtime_error("Failed to open input file '" + _fileB + "'");

    // set up input filters, keep SNV only, and reject entries with N ref value
    NoReferenceFilter nref;
    TypeFilter snvOnly(Bed::SNV);

    BedStream fa(_fileA, inA);
    fa.addFilter(&snvOnly);
    fa.addFilter(&nref);

    BedStream fb(_fileB, inB);
    fb.addFilter(&snvOnly);

    ConcordanceQuality qc;
    ResultMultiplexer rmux;
    rmux.add(&qc);

    auto_ptr<ResultStreamWriter> resultStreamWriter = setupStreamWriter();
    if (resultStreamWriter.get())
        rmux.add(resultStreamWriter.get());
    SnvComparator snvi(fa, fb, rmux);
    snvi.exec();
    qc.report(cout); 

    cout << "Total Snvs: " << fa.bedCount() << endl;
    cout << "      Hits: " << qc.hits() << endl;
    cout << "    Misses: " << qc.misses() << endl;
}