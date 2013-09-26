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
using namespace std::placeholders;
namespace po = boost::program_options;

CommandBase::ptr Vcf2RawCommand::create(int argc, char** argv) {
    boost::shared_ptr<Vcf2RawCommand> app(new Vcf2RawCommand);
    app->parseArguments(argc, argv);
    return app;
}

Vcf2RawCommand::Vcf2RawCommand()
    : _outFile("-")
{
}

void Vcf2RawCommand::parseArguments(int argc, char** argv) {
    po::options_description opts("Available Options");
    opts.add_options()
        ("help,h", "this message")
        ("vcf,v", po::value<string>(&_vcfFile), "input .vcf file (required, - for stdin)")
        ("fasta,f", po::value<string>(&_refFa), "reference sequence file (fasta format)")
        ("output,o", po::value<string>(&_outFile), "output .bed file (- or omit for stdout)")
    ;

    po::positional_options_description posOpts;
    posOpts.add("vcf", 1);
    posOpts.add("output", 1);

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
        throw CmdlineHelpException(ss.str());
    }

    if (!vm.count("vcf")) {
        stringstream ss;
        ss << "no vcf file specified!" << endl << endl << opts;
        throw runtime_error(ss.str());
    }

    if (!vm.count("fasta")) {
        stringstream ss;
        ss << "no fasta file specified!" << endl << endl << opts;
        throw runtime_error(ss.str());
    }
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
    InputStream::ptr in = _streams.wrap<istream, InputStream>(_vcfFile);
    VcfReader::ptr vcfReader = openVcf(*in);
    OutputWriter writer(*out, ref);
    VcfToRaw<VcfReader, OutputWriter> converter(*vcfReader, writer);
    converter.convert();
}
