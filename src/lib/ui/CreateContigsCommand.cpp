#include "CreateContigsCommand.hpp"

#include "common/UnknownSequenceError.hpp"
#include "fileformats/Fasta.hpp"
#include "fileformats/TypedStream.hpp"
#include "fileformats/vcf/CustomValue.hpp"
#include "fileformats/vcf/Entry.hpp"
#include "fileformats/vcf/Header.hpp"
#include "fileformats/vcf/RawVariant.hpp"
#include "io/InputStream.hpp"
#include "processors/VariantContig.hpp"

#include <boost/format.hpp>

#include <fstream>
#include <memory>
#include <stdexcept>

using boost::format;
using namespace std;
namespace po = boost::program_options;

CreateContigsCommand::CreateContigsCommand()
    : _outputFasta("-")
    , _flankSize(99)
    , _minQuality(0)
{
}

void CreateContigsCommand::configureOptions() {
    _opts.add_options()

        ("reference,r",
            po::value<string>(&_referenceFasta)->required(),
            "input reference sequence (FASTA format)")

        ("variants,v",
            po::value<string>(&_variantsFile)->required(),
            "input variants file (.vcf format)")

        ("output-fasta,o",
            po::value<string>(&_outputFasta)->default_value("-"),
            "output fasta (empty or - means stdout, which is the default)")

        ("output-remap,R",
            po::value<string>(&_outputRemap)->required(),
            "output remap")

        ("flank,f",
            po::value<int>(&_flankSize)->default_value(99),
            "flank size on either end of the variant (default=99)")

        ("quality,q",
            po::value<int>(&_minQuality)->default_value(0),
            "minimum quality cutoff for variants (default=0)")
    ;

    _posOpts.add("reference", 1);
    _posOpts.add("variants", 1);
}

void CreateContigsCommand::exec() {
    Fasta ref(_referenceFasta);
    ostream *outputFasta = _streams.get<ostream>(_outputFasta);
    ostream *outputRemap = _streams.get<ostream>(_outputRemap);

    InputStream::ptr in(_streams.openForReading(_variantsFile));
    auto vcfReader = openStream<Vcf::Entry>(in);
    auto& reader = *vcfReader;

    Vcf::Entry entry;
    while (reader.next(entry)) {
        if (entry.identifiers().empty())
            continue;

        Vcf::RawVariant::Vector variants = Vcf::RawVariant::processEntry(entry);
        for (auto i = variants.begin(); i != variants.end(); ++i) {
            size_t idx = distance(variants.begin(), i);
            stringstream namestream;
            namestream << *entry.identifiers().begin() << "_" << entry.pos() << "_" << idx;
            try {
                VariantContig contig(*i, ref, _flankSize, entry.chrom());
                string name = namestream.str();
                *outputFasta << ">" << name << "\n" << contig.sequence() << "\n";
                *outputRemap << ">" << name << "-"
                    << entry.chrom() << "|"
                    << contig.start() << "|"
                    << contig.stop() << "\n"
                    << contig.cigar() << "\n";
            } catch (UnknownSequenceError& e) {
                cerr << "WARNING: at line " << reader.name() << ":" << reader.lineNum() << ": "
                    << e.what() << "\n";
            }
        }
    }

}
