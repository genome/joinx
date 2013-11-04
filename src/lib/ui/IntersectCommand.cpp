#include "IntersectCommand.hpp"
#include "IntersectCollector.hpp"

#include "common/cstdint.hpp"
#include "fileformats/BedReader.hpp"
#include "processors/IntersectFull.hpp"
#include "processors/IntersectionOutputFormatter.hpp"

#include <boost/assign/list_of.hpp>
#include <boost/bind.hpp>
#include <boost/format.hpp>
#include <boost/function.hpp>
#include <boost/program_options.hpp>

#include <algorithm>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <iterator>
#include <memory>
#include <sstream>
#include <stdexcept>

namespace po = boost::program_options;
using boost::format;
using namespace std;

IntersectCommand::IntersectCommand()
    : _outputFile("-")
    , _formatString("A")
    , _firstOnly(false)
    , _outputBoth(false)
    , _exactPos(false)
    , _exactAllele(false)
    , _iubMatch(false)
    , _dbsnpMatch(false)
    , _adjacentInsertions(false)
{
}

void IntersectCommand::configureOptions() {
    _opts.add_options()
        ("file-a,a",
            po::value<string>(&_fileA)->required(),
            "input .bed file a (required, - for stdin)")

        ("file-b,b",
            po::value<string>(&_fileB)->required(),
            "input .bed file b (required, - for stdin)")

        ("output-file,o",
            po::value<string>(&_outputFile)->default_value("-"),
            "output file (empty or - means stdout)")

        ("miss-a", po::value<string>(&_missFileA), "output misses in A to file")

        ("miss-b", po::value<string>(&_missFileB), "output misses in B to file")

        ("format-string,F",
            po::value<string>(&_formatString),
            "specify the output format explicity (see man page).")

        ("full", "'full' output format, equivalent to -F 'I A B'")

        ("first-only,f",
            po::bool_switch(&_firstOnly),
            "report only the first thing to hit records in b, not the full intersection")

        ("output-both", "concatenate intersecting lines in output (vs writing out only lines from 'a')")

        ("exact-pos",
            po::bool_switch(&_exactPos),
            "require exact match of coordinates (default is to count overlaps)")

        ("exact-allele", "require exact match of coordinates AND allele values")

        ("iub-match",
            po::bool_switch(&_iubMatch),
            "when using --exact-allele, this enables expansion and partial matching of IUB codes")

        ("dbsnp-match", "like using --iub-match, but will try to reverse the reference and variant in file b, as well as reverse complement to get a match")

        ("adjacent-insertions",
            po::bool_switch(&_adjacentInsertions),
            "count insertions adjacent to other regions as intersecting")
        ;

    _posOpts.add("file-a", 1);
    _posOpts.add("file-b", 1);
}

void IntersectCommand::finalizeOptions() {
   // check for mutually exclusive formatting options
    vector<string> exclusiveFormattingOpts = boost::assign::list_of
        ("output-both")
        ("format-string")
        ("full");

    vector<string> formattingOpts;
    for (auto i = exclusiveFormattingOpts.begin(); i != exclusiveFormattingOpts.end(); ++i) {
        if (_varMap.count(*i))
            formattingOpts.push_back(*i);
    }
    if (formattingOpts.size() > 1) {
        stringstream opts;
        copy(formattingOpts.begin(), formattingOpts.end(), ostream_iterator<string>(opts, " "));

        throw runtime_error(
            str(format("Mutually exclusive formatting options detected, please specify only one of: %s") %opts.str())
        );
    }

    if (_varMap.count("output-both")) {
        _formatString = "A B";
        _outputBoth = true;
    }

    if (_varMap.count("full")) {
        _formatString = "I A B";
        _outputBoth = true;
    }

    if (_varMap.count("exact-allele")) {
        _exactAllele = true;
        _exactPos = true;
    }

    if (_varMap.count("dbsnp-match")) {
        _exactAllele = true;
        _exactPos = true;
        _iubMatch = true;
        _dbsnpMatch = true;
    }
}

void IntersectCommand::exec() {
    if (_fileA == _fileB)
        throw runtime_error("Input files have the same name, '" + _fileA + "', not good.");

    ostream* outHit = _streams.get<ostream>(_outputFile);
    IntersectionOutput::Formatter outputFormatter(_formatString, *outHit);

    // determine how many extra (beyond the 3 mandatory) fields we need to
    // parse. this depends on how many we are expected to print, let's ask
    // the outputFormatter!
    unsigned extraFieldsA = max(1u, outputFormatter.extraFields(0));
    unsigned extraFieldsB = max(1u, outputFormatter.extraFields(1));
    InputStream::ptr inStreamA(_streams.openForReading(_fileA));
    BedReader::ptr readerPtrA = openBed(*inStreamA, extraFieldsA);
    auto& fa = *readerPtrA;

    InputStream::ptr inStreamB(_streams.openForReading(_fileB));
    BedReader::ptr readerPtrB = openBed(*inStreamB, extraFieldsB);
    auto& fb = *readerPtrB;

    // don't try to read cin more than once or you will have a bad day
    if (_streams.cinReferences() > 1)
        throw runtime_error("Multiple input streams from stdin specified. Abort.");

    // optional "miss" output streams
    ostream* outMissA(NULL);
    ostream* outMissB(NULL);
    if (!_missFileA.empty()) outMissA = _streams.get<ostream>(_missFileA);
    if (!_missFileB.empty()) outMissB = _streams.get<ostream>(_missFileB);

    IntersectCollector c(_outputBoth, _exactPos, _exactAllele, _iubMatch, _dbsnpMatch, outputFormatter, outMissA, outMissB);
    IntersectFull<BedReader, BedReader, IntersectCollector> intersector(fa, fb, c, _adjacentInsertions);
    intersector.execute();
}
