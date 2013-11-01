#include "VcfNormalizeIndelsCommand.hpp"

#include "common/UnknownSequenceError.hpp"
#include "fileformats/Fasta.hpp"
#include "fileformats/InputStream.hpp"
#include "fileformats/OutputWriter.hpp"
#include "fileformats/VcfReader.hpp"
#include "fileformats/vcf/Entry.hpp"
#include "fileformats/vcf/Header.hpp"
#include "fileformats/vcf/AltNormalizer.hpp"

#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <functional>
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
    InputStream::ptr inStream = _streams.wrap<istream, InputStream>(_inputFile);
    ostream* out = _streams.get<ostream>(_outputFile);
    if (_streams.cinReferences() > 1)
        throw runtime_error("stdin listed more than once!");

    VcfReader::ptr in = openVcf(*inStream);
    *out << in->header();
    Vcf::Entry e;
    Vcf::AltNormalizer norm(ref);
    while (in->next(e)) {
        try {
            norm.normalize(e);
        } catch (UnknownSequenceError const& ex) {
            // We couldn't get reference data for the sequence
            cerr << "WARNING: at line " << in->lineNum() << " in file " << in->name()
                << ": sequence " << e.chrom() << " not found in reference " << _fastaPath << "\n";
        }
        *out << e << "\n";
    }
}
