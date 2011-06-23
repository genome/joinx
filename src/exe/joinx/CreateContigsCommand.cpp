#include "CreateContigsCommand.hpp"

#include "bedutil/VariantsToContigs.hpp"
#include "fileformats/FastaReader.hpp"
#include "fileformats/BedStream.hpp"

#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <fstream>
#include <memory>
#include <stdexcept>

using boost::format;
using namespace std;
namespace po = boost::program_options;

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
    std::istream* input = &cin; 
    std::ostream* output = &cout;
    if (!_variantsFile.empty() && _variantsFile != "-") {
        input = new ifstream(_variantsFile);
        if (!*input)
            throw runtime_error(str(format("Failed to open variants file %1%") %_variantsFile));
    }

    if (!_outputFile.empty() && _outputFile != "-") {
        output = new ofstream(_outputFile);
        if (!*output)
            throw runtime_error(str(format("Failed to open output file %1%") %_outputFile));
    }

    // this stream will read 2 extra fields, ref/call and quality
    BedStream bedStream(_variantsFile, *input, 2);

    VariantsToContigs<FastaReader,BedStream> v2c(
        ref, bedStream, *output, _flankSize, _minQuality
    );
    v2c.execute();

    if (input != &cin) delete input;
    if (output != &cout) delete output;
}
