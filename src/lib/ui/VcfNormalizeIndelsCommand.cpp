#include "VcfNormalizeIndelsCommand.hpp"

#include "common/UnknownSequenceError.hpp"
#include "fileformats/Fasta.hpp"
#include "fileformats/TypedStream.hpp"
#include "fileformats/vcf/Entry.hpp"
#include "fileformats/vcf/Header.hpp"
#include "fileformats/vcf/AltNormalizer.hpp"

#include <boost/format.hpp>

#include <unordered_set>
#include <stdexcept>

namespace po = boost::program_options;
using boost::format;
using namespace std;

VcfNormalizeIndelsCommand::VcfNormalizeIndelsCommand()
    : _outputFile("-")
    , _clearFilters(false)
    , _mergeSamples(false)
{
}

void VcfNormalizeIndelsCommand::configureOptions() {
    _opts.add_options()
        ("input-file,i",
            po::value<string>(&_inputFile)->default_value("-"),
            "input file ('-' for stdin)")

        ("fasta,f",
            po::value<string>(&_fastaPath)->required(),
            "reference sequence (fasta)")

        ("output-file,o",
            po::value<string>(&_outputFile),
            "output file (omit or use '-' for stdout)")
        ;

    _posOpts.add("input-file", -1);
}

void VcfNormalizeIndelsCommand::exec() {
    Fasta ref(_fastaPath);
    auto in = _streams.openForReading(_inputFile);
    ostream* out = _streams.get<ostream>(_outputFile);
    if (_streams.cinReferences() > 1)
        throw runtime_error("stdin listed more than once!");

    auto reader = openStream<Vcf::Entry>(in);
    *out << reader->header();
    Vcf::Entry e;
    Vcf::AltNormalizer norm(ref);
    std::unordered_set<std::string> seqWarnings;
    while (reader->next(e)) {
        try {
            norm.normalize(e);
        } catch (UnknownSequenceError const& ex) {
            auto inserted = seqWarnings.insert(e.chrom());
            // only warn the first time for each sequence
            if (inserted.second) {
                // We couldn't get reference data for the sequence
                cerr << "WARNING: at line " << reader->lineNum()
                    << " in file " << reader->name() << ": sequence "
                    << e.chrom() << " not found in reference " << _fastaPath << "\n"
                    ;
            }
        }
        *out << e << "\n";
    }
}
