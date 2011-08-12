#include "CreateContigsCommand.hpp"

#include "bedutil/RemappedContig.hpp"
#include "fileformats/BedStream.hpp"
#include "fileformats/FastaReader.hpp"
#include "fileformats/InputStream.hpp"

#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <fstream>
#include <memory>
#include <stdexcept>

using boost::format;
using namespace std;
namespace po = boost::program_options;

namespace {
    class RemappedContigFastaWriter {
    public:
        RemappedContigFastaWriter(ostream& out)
            : _out(out)
        {
        }

        void operator()(const RemappedContig& ctg) {
            _out << ">" << ctg.name() << "\n" << ctg.sequence() << "\n";
        }

    protected:
        ostream& _out;
    };
}

CreateContigsCommand::CreateContigsCommand()
    : _flankSize(99)
    , _minQuality(0)
{
}

CommandBase::ptr CreateContigsCommand::create(int argc, char** argv) {
    boost::shared_ptr<CreateContigsCommand> app(new CreateContigsCommand);
    app->parseArguments(argc, argv);
    return app;
}

void CreateContigsCommand::parseArguments(int argc, char** argv) {
    po::options_description opts("Available Options");
    opts.add_options()
        ("help,h", "this message")
        ("reference,r", po::value<string>(&_referenceFasta), "input reference sequence (FASTA format)")
        ("variants,v", po::value<string>(&_variantsFile), "input variants file (.bed format)")
        ("output-file,o", po::value<string>(&_outputFile), "output file (empty or - means stdout, which is the default)")
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
}

void CreateContigsCommand::exec() {
    FastaReader ref(_referenceFasta);
    ostream *output = _streams.get<ostream>(_outputFile);

    InputStream::ptr inStream(_streams.wrap<istream, InputStream>(_variantsFile));
    // this stream will read 2 extra fields, ref/call and quality
    BedStream bedStream(*inStream, 2);
    RemappedContigFastaWriter writer(*output);
    RemappedContigGenerator<FastaReader, RemappedContigFastaWriter> generator(ref, _flankSize, writer);
    Bed b;
    while (bedStream.next(b)) {
        Variant v(b);
        if (v.quality() >= _minQuality)
            generator.generate(v);
    }

}
