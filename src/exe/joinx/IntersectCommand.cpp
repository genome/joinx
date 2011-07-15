#include "IntersectCommand.hpp"
#include "Collector.hpp"

#include "bedutil/IntersectionOutputFormatter.hpp"
#include "bedutil/Intersect.hpp"
#include "common/intconfig.hpp"
#include "fileformats/BedStream.hpp"

#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <algorithm>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <iterator>
#include <memory>
#include <sstream>
#include <stdexcept>

using boost::format;
using namespace std;
namespace po = boost::program_options;

CommandBase::ptr IntersectCommand::create(int argc, char** argv) {
    boost::shared_ptr<IntersectCommand> app(new IntersectCommand);
    app->parseArguments(argc, argv);
    return app;
}

IntersectCommand::IntersectCommand()
    : _outputFile("-")
    , _formatString("A")
    , _firstOnly(false)
    , _outputBoth(false)
    , _exactPos(false)
    , _exactAllele(false)
    , _iubMatch(false)
    , _dbsnpMatch(false)
{
}

void IntersectCommand::parseArguments(int argc, char** argv) {
    po::options_description opts("Available Options");
    opts.add_options()
        ("help,h", "this message")
        ("file-a,a", po::value<string>(&_fileA), "input .bed file a (required, - for stdin)")
        ("file-b,b", po::value<string>(&_fileB), "input .bed file b (required, - for stdin)")
        ("output-file,o", po::value<string>(&_outputFile), "output file (empty or - means stdout, which is the default)")
        ("miss-a", po::value<string>(&_missFileA), "output misses in A to file")
        ("miss-b", po::value<string>(&_missFileB), "output misses in B to file")
        ("format-string,F", po::value<string>(&_formatString), "specify the output format explicity (see man page).")
        ("full", "'full' output format, equivalent to -F 'I A B'")
        ("first-only,f", "notice only the first thing to hit records in b, not the full intersection")
        ("output-both", "concatenate intersecting lines in output (vs writing out only lines from 'a')")
        ("exact-pos", "require exact match of coordinates (default is to count overlaps)")
        ("exact-allele", "require exact match of coordinates AND allele values")
        ("iub-match", "when using --exact-allele, this enables expansion and partial matching of IUB codes")
        ("dbsnp-match", "like using --iub-match, but will try to reverse the reference and variant in file b, as well as reverse complement to get a match");

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

    // check for mutually exclusive formatting options
    vector<string> exclusiveFormattingOpts = {
        "output-both",
        "format-string",
        "full",
    };
    vector<string> formattingOpts;
    for (auto i = exclusiveFormattingOpts.begin(); i != exclusiveFormattingOpts.end(); ++i) {
        if (vm.count(*i))
            formattingOpts.push_back(*i);
    }
    if (formattingOpts.size() > 1) {
        stringstream opts;
        copy(formattingOpts.begin(), formattingOpts.end(), ostream_iterator<string>(opts, " "));

        throw runtime_error(
            str(format("Mutually exclusive formatting options detected, please specify only one of: %s") %opts.str())
        );
    }

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

    if (vm.count("output-both")) {
        _formatString = "A B";
        _outputBoth = true;
    }

    if (vm.count("full")) {
        _formatString = "I A B";
        _outputBoth = true;
    }

    if (vm.count("exact-pos"))
        _exactPos = true;

    if (vm.count("exact-allele")) {
        _exactAllele = true;
        _exactPos = true;
    }

    if (vm.count("iub-match")) {
        _iubMatch = true;
    }

    if (vm.count("dbsnp-match")) {
        _exactAllele = true;
        _exactPos = true;
        _iubMatch = true;
        _dbsnpMatch = true;
    }
}

void IntersectCommand::setupStreams(Streams& s) {
    unsigned cinReferences = 0;

    if (_fileA != "-") {
        s.inA = _streams.get(_fileA, ios::in);
    } else {
        s.inA = &cin;
        ++cinReferences;
    }

    if (_fileB != "-") {
        s.inB = _streams.get(_fileB, ios::in);
    } else {
        s.inB = &cin;
        ++cinReferences;
    }

    if (cinReferences > 1)
        throw runtime_error("Multiple input streams from stdin specified. Abort.");

    if (!_outputFile.empty() && _outputFile != "-") {
        s.outHit = _streams.get(_outputFile, ios::out);
    } else {
        s.outHit = &cout;
    }

    if (!_missFileA.empty() && _missFileA != "-") {
        s.outMissA = _streams.get(_missFileA, ios::out);
    } else if (!_missFileA.empty()) {
        s.outMissA = &cout;
    }

    if (!_missFileB.empty() && _missFileB != "-") {
        s.outMissB = _streams.get(_missFileB, ios::out);
    } else if (!_missFileB.empty()) {
        s.outMissB = &cout;
    }
}

void IntersectCommand::exec() {
    if (_fileA == _fileB) {
        throw runtime_error("Input files have the same name, '" + _fileA + "', not good.");
    }

    Streams s;
    setupStreams(s);

    IntersectionOutput::Formatter outputFormatter(_formatString, *s.outHit);

    unsigned extraFieldsA = max(1u, outputFormatter.extraFields(0));
    unsigned extraFieldsB = max(1u, outputFormatter.extraFields(1));
    //
    BedStream fa(_fileA, *s.inA, extraFieldsA);
    BedStream fb(_fileB, *s.inB, extraFieldsB);

    Collector c(_outputBoth, _exactPos, _exactAllele, _iubMatch, _dbsnpMatch, outputFormatter, s.outMissA, s.outMissB);
    Intersect<BedStream,BedStream,Collector> intersector(fa, fb, c);
    intersector.execute();
}
