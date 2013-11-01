#include "CreateContigsCommand.hpp"

#include "common/UnknownSequenceError.hpp"
#include "fileformats/Fasta.hpp"
#include "fileformats/InputStream.hpp"
#include "fileformats/TypedStream.hpp"
#include "fileformats/VcfReader.hpp"
#include "fileformats/vcf/CustomValue.hpp"
#include "fileformats/vcf/Entry.hpp"
#include "fileformats/vcf/RawVariant.hpp"
#include "processors/VariantContig.hpp"

#include <boost/assign/list_of.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>

#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <fstream>
#include <memory>
#include <stdexcept>

using boost::assign::list_of;
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

    InputStream::ptr instream(_streams.wrap<istream, InputStream>(_variantsFile));

    typedef boost::function<void(const Vcf::Header*, string&, Vcf::Entry&)> VcfExtractor;
    typedef TypedStream<Vcf::Entry, VcfExtractor> ReaderType;
    typedef boost::shared_ptr<ReaderType> ReaderPtr;

    VcfExtractor extractor = boost::bind(&Vcf::Entry::parseLine, _1, _2, _3);
    ReaderType reader(extractor, *instream);
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
