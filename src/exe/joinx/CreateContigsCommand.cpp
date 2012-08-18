#include "CreateContigsCommand.hpp"

#include "common/UnknownSequenceError.hpp"
#include "fileformats/Fasta.hpp"
#include "fileformats/InputStream.hpp"
#include "fileformats/TypedStream.hpp"
#include "fileformats/VcfReader.hpp"
#include "fileformats/vcf/Entry.hpp"
#include "fileformats/vcf/RawVariant.hpp"
#include "processors/VariantContig.hpp"

#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <fstream>
#include <functional>
#include <memory>
#include <stdexcept>

using boost::format;
using namespace std;
using namespace std::placeholders;
namespace po = boost::program_options;

CreateContigsCommand::CreateContigsCommand()
    : _outputFasta("-")
    , _flankSize(99)
    , _minQuality(0)
{
}

CommandBase::ptr CreateContigsCommand::create(int argc, char** argv) {
    std::shared_ptr<CreateContigsCommand> app(new CreateContigsCommand);
    app->parseArguments(argc, argv);
    return app;
}

void CreateContigsCommand::parseArguments(int argc, char** argv) {
    po::options_description opts("Available Options");
    opts.add_options()
        ("help,h", "this message")
        ("reference,r", po::value<string>(&_referenceFasta), "input reference sequence (FASTA format)")
        ("variants,v", po::value<string>(&_variantsFile), "input variants file (.bed format)")
        ("output-fasta,o", po::value<string>(&_outputFasta), "output fasta (empty or - means stdout, which is the default)")
        ("output-remap,R", po::value<string>(&_outputRemap), "output remap")
        ("flank,f", po::value<int>(&_flankSize), "flank size on either end of the variant (default=99)")
        ("quality,q", po::value<int>(&_minQuality), "minimum quality cutoff for variants (default=0)")
    ;

    po::positional_options_description posOpts;
    posOpts.add("reference", 1);
    posOpts.add("variants", 1);

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

    if (!vm.count("reference")) {
        stringstream ss;
        ss << "no reference specified!" << endl << endl << opts;
        throw runtime_error(ss.str());
    }

    if (!vm.count("variants")) {
        stringstream ss;
        ss << "no variants specified!" << endl << endl << opts;
        throw runtime_error(ss.str());
    }

    if (vm.count("output-fasta") != 1) {
        throw runtime_error("specify --output-fasta= exactly once");
    }

    if (vm.count("output-remap") != 1) {
        throw runtime_error("specify --output-remap= exactly once");
    }
}

void CreateContigsCommand::exec() {
    Fasta ref(_referenceFasta);
    ostream *outputFasta = _streams.get<ostream>(_outputFasta);
    ostream *outputRemap = _streams.get<ostream>(_outputRemap);

    InputStream::ptr instream(_streams.wrap<istream, InputStream>(_variantsFile));

    typedef function<void(const Vcf::Header*, string&, Vcf::Entry&)> VcfExtractor;
    typedef TypedStream<Vcf::Entry, VcfExtractor> ReaderType;
    typedef shared_ptr<ReaderType> ReaderPtr;

    VcfExtractor extractor = bind(&Vcf::Entry::parseLine, _1, _2, _3);
    ReaderType reader(extractor, *instream);
    Vcf::Entry entry;
    while (reader.next(entry)) {
        vector<Vcf::RawVariant> variants = Vcf::RawVariant::processEntry(entry);
        for (auto i = variants.begin(); i != variants.end(); ++i) {
            if (entry.identifiers().empty()) {
                continue;
            }
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
