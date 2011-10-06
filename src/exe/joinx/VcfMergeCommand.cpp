#include "VcfMergeCommand.hpp"

#include "bedutil/MergeSorted.hpp"
#include "fileformats/InputStream.hpp"
#include "fileformats/OutputWriter.hpp"
#include "fileformats/TypedStream.hpp"
#include "fileformats/vcf/Builder.hpp"
#include "fileformats/vcf/Entry.hpp"
#include "fileformats/vcf/Header.hpp"
#include "fileformats/vcf/MergeStrategy.hpp"

#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <cstdint>
#include <functional>
#include <stdexcept>

namespace po = boost::program_options;
using boost::format;
using namespace std;
using namespace std::placeholders;

CommandBase::ptr VcfMergeCommand::create(int argc, char** argv) {
    std::shared_ptr<VcfMergeCommand> app(new VcfMergeCommand);
    app->parseArguments(argc, argv);
    return app;
}

VcfMergeCommand::VcfMergeCommand()
    : _outputFile("-")
{
}

void VcfMergeCommand::parseArguments(int argc, char** argv) {
    po::options_description opts("Available Options");
    opts.add_options()
        ("help,h", "this message")
        ("input-file,i", po::value< vector<string> >(&_filenames), "input file(s) (empty or - means stdin, which is the default)")
        ("output-file,o", po::value<string>(&_outputFile), "output file (empty or - means stdout, which is the default)")
        ;
    po::positional_options_description posOpts;
    posOpts.add("input-file", -1);

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

    if (!vm.count("input-file"))
        _filenames.push_back("-");
}

namespace {

}

void VcfMergeCommand::exec() {
    vector<InputStream::ptr> inputStreams = _streams.wrap<istream, InputStream>(_filenames);
    ostream* out = _streams.get<ostream>(_outputFile);
    if (_streams.cinReferences() > 1)
        throw runtime_error("stdin listed more than once!");

    typedef function<void(string&, Vcf::Entry&)> VcfExtractor;
    typedef TypedStream<Vcf::Entry, VcfExtractor> ReaderType;
    typedef shared_ptr<ReaderType> ReaderPtr;
    typedef OutputWriter<Vcf::Entry> WriterType;

    vector<ReaderPtr> readers;
    vector<Vcf::Header> headers;
    vector<VcfExtractor> extractors;
    for (auto i = inputStreams.begin(); i != inputStreams.end(); ++i)
        headers.push_back(Vcf::Header::fromStream(**i));

    for (size_t i = 0; i < headers.size(); ++i) {
        extractors.push_back(bind(&Vcf::Entry::parseLine, &headers[i], _1, _2));
        readers.push_back(ReaderPtr(new ReaderType(extractors[i], *inputStreams[i])));
    }

    Vcf::Header mergedHeader = headers[0];
    for (auto i = headers.begin() + 1; i != headers.end(); ++i)
        mergedHeader.merge(*i);

    WriterType writer(*out);
    Vcf::MergeStrategy mergeStrategy(&mergedHeader);
    Vcf::Builder builder(mergeStrategy, &mergedHeader, writer);
    *out << mergedHeader;
    MergeSorted<Vcf::Entry, ReaderPtr, Vcf::Builder> merger(readers, builder);
    merger.execute();
    builder.flush();
}
