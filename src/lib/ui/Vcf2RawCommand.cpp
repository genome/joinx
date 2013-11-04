#include "Vcf2RawCommand.hpp"

#include "fileformats/Fasta.hpp"
#include "fileformats/VcfReader.hpp"
#include "fileformats/vcf/AltNormalizer.hpp"
#include "fileformats/vcf/RawVariant.hpp"
#include "processors/VcfToRaw.hpp"

#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <functional>

using boost::format;
using namespace std;
namespace po = boost::program_options;

Vcf2RawCommand::Vcf2RawCommand()
    : _outFile("-")
{
}

void Vcf2RawCommand::configureOptions() {
    _opts.add_options()
        ("vcf,v",
            po::value<string>(&_vcfFile)->required(),
            "input .vcf file (required)")

        ("fasta,f",
            po::value<string>(&_refFa)->required(),
            "reference sequence file (fasta format, required)")

        ("output,o",
            po::value<string>(&_outFile)->default_value("-"),
            "output .bed file (- or omit for stdout)")
    ;

    _posOpts.add("vcf", 1);
    _posOpts.add("output", 1);

}

namespace {
    struct OutputWriter {
        OutputWriter(ostream& out, Fasta& ref)
            : out(out)
            , _ref(ref)
        {
        }

        void operator()(std::string const& chrom, Vcf::RawVariant& raw) {
            if (chrom != _chr) {
                size_t len = _ref.seqlen(chrom);
                _sequence = _ref.sequence(chrom, 1, len);
                _chr = chrom;
            }
            normalizeRaw(raw, _sequence);

            out << chrom << "\t"
                << raw.pos << "\t"
                << (raw.ref.empty() ? "*" : raw.ref) << "/"
                << (raw.alt.empty() ? "*" : raw.alt) << "\t"
                << "\n";
        }

        ostream& out;
        Fasta& _ref;
        string _chr;
        string _sequence;
    };
}

void Vcf2RawCommand::exec() {
    Fasta ref(_refFa);

    ostream* out = _streams.get<ostream>(_outFile);
    auto in = _streams.openForReading(_vcfFile);
    VcfReader::ptr vcfReader = openVcf(*in);
    OutputWriter writer(*out, ref);
    VcfToRaw<VcfReader, OutputWriter> converter(*vcfReader, writer);
    converter.convert();
}
