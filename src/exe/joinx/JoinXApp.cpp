#include "JoinXApp.hpp"
#include "Collector.hpp"

#include "bedutil/Intersect.hpp"
#include "common/intconfig.hpp"
#include "fileformats/BedStream.hpp"

#include <boost/program_options.hpp>
#include <algorithm>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>

using namespace std;
namespace po = boost::program_options;

JoinXApp::JoinXApp(int argc, char** argv)
    : _outputFile("-")
    , _firstOnly(false)
    , _outputBoth(false)
    , _exactPos(false)
    , _exactAllele(false)
{
    parseArguments(argc, argv);
}

void JoinXApp::parseArguments(int argc, char** argv) {
    po::options_description opts("Available Options");
    opts.add_options()
        ("help,h", "this message")
        ("file-a,a", po::value<string>(&_fileA), "input .bed file a (required, - for stdin)")
        ("file-b,b", po::value<string>(&_fileB), "input .bed file b (required, - for stdin)")
        ("output-file,o", po::value<string>(&_outputFile), "output file (empty or - means stdout, which is the default)")
        ("miss-a", po::value<string>(&_missFileA), "output misses in A to file")
        ("miss-b", po::value<string>(&_missFileB), "output misses in B to file")
        ("first-only,f", "notice only the first thing to hit records in b, not the full intersection")
        ("output-both", "concatenate intersecting lines in output (vs writing out only lines from 'a')")
        ("exact-pos", "require exact match of coordinates (default is to count overlaps)")
        ("exact-allele", "require exact match of coordinates AND allele values");

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

    if (vm.count("first-only"))
        _firstOnly = true;
    
    if (vm.count("output-both"))
        _outputBoth = true;

    if (vm.count("exact-pos"))
        _exactPos = true;

    if (vm.count("exact-allele")) {
        _exactAllele = true;
        _exactPos = true;
    }
}

void JoinXApp::setupStreams(Streams& s) const {
    unsigned cinReferences = 0;
    unsigned coutReferences = 0;

    fstream* fs;

    if (_fileA != "-") {
        s.inA = fs = new fstream(_fileA.c_str(), ios::in);
        if (!*s.inA)
            throw runtime_error("Failed to open input file '" + _fileA + "'");
        s.cleanup.push_back(fs);
    } else {
        s.inA = &cin;
        ++cinReferences;
    }

    if (_fileB != "-") {
        s.inB = fs = new fstream(_fileB.c_str(), ios::in);
        if (!*s.inB)
            throw runtime_error("Failed to open input file '" + _fileB + "'");
        s.cleanup.push_back(fs);
    } else {
        s.inB = &cin;
        ++cinReferences;
    }

    if (cinReferences > 1)
        throw runtime_error("Multiple input streams from stdin specified. Abort.");

    if (!_outputFile.empty() && _outputFile != "-") {
        s.outHit = fs = new fstream(_outputFile.c_str(), ios::out);
        if (!*s.outHit)
            throw runtime_error("Failed to open output file '" + _outputFile + "'");
        s.cleanup.push_back(fs);
    } else {
        s.outHit = &cout;
        ++coutReferences; 
    }

    if (!_missFileA.empty() && _missFileA != "-") {
        s.outMissA = fs = new fstream(_missFileA.c_str(), ios::out);
        if (!*s.outMissA)
            throw runtime_error("failed to open output file '" + _missFileA + "'");
        s.cleanup.push_back(fs);
    } else if (!_missFileA.empty()) {
        s.outMissA = &cout;
        ++coutReferences;
    }

    if (!_missFileB.empty() && _missFileB != "-") {
        s.outMissB = fs = new fstream(_missFileB.c_str(), ios::out);
        if (!*s.outMissB)
            throw runtime_error("failed to open output file '" + _missFileB + "'");
        s.cleanup.push_back(fs);
    } else if (!_missFileB.empty()) {
        s.outMissB = &cout;
        ++coutReferences;
    }

    if (coutReferences > 1)
        throw runtime_error("Multiple output streams to stdout specified. Abort.");
}

void JoinXApp::exec() {
    if (_fileA == _fileB) {
        throw runtime_error("Input files have the same name, '" + _fileA + "', not good.");
    }

    Streams s;
    setupStreams(s);

    // these bedstreams will read 1 extra field, which is ref/call
    BedStream fa(_fileA, *s.inA, 1);
    BedStream fb(_fileB, *s.inB, 1);
    Collector c(_outputBoth, _exactPos, _exactAllele, *s.outHit, s.outMissA, s.outMissB);

    Intersect<BedStream,BedStream,Collector> intersector(fa, fb, c);

    intersector.execute();
}
