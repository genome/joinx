#include "VcfAnnotateCommand.hpp"

#include "bedutil/Intersect.hpp"
#include "fileformats/Bed.hpp"
#include "fileformats/InputStream.hpp"
#include "fileformats/OutputWriter.hpp"
#include "fileformats/TypedStream.hpp"
#include "fileformats/vcf/Entry.hpp"
#include "fileformats/vcf/Header.hpp"

#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <cstdint>
#include <functional>
#include <stdexcept>

namespace po = boost::program_options;
using boost::format;
using namespace std;
using namespace std::placeholders;

CommandBase::ptr VcfAnnotateCommand::create(int argc, char** argv) {
    std::shared_ptr<VcfAnnotateCommand> app(new VcfAnnotateCommand);
    app->parseArguments(argc, argv);
    return app;
}

VcfAnnotateCommand::VcfAnnotateCommand()
    : _outputFile("-")
{
}

void VcfAnnotateCommand::parseArguments(int argc, char** argv) {
    po::options_description opts("Available Options");
    opts.add_options()
        ("help,h", "this message")
        ("input-file,i", po::value< vector<string> >(&_filenames), "input file(s) (positional arguments work also)")
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
template<typename OutputType>
class Catcher {
public:
    Catcher(OutputType& out)
        : _out(out)
    {
    }

    bool wantMissA() const { return true; }
    bool wantMissB() const { return false; }

    template<typename TypeA, typename TypeB>
    bool hit(const TypeA& a, const TypeB& b) {
        TypeA copyA(a);
        copyA.addIdentifier(b.extraFields()[4]);
        _out(copyA);
        return true;
    }

    template<typename TypeA>
    void missA(const TypeA& a) {
        _out(a);
    }

    template<typename TypeB>
    void missB(const TypeB& b) {
    }

protected:
    OutputType& _out;
};
}

void VcfAnnotateCommand::exec() {
    if (_filenames.size() != 2) {
        throw runtime_error("2 files please.");
    }

    vector<InputStream::ptr> inputStreams = _streams.wrap<istream, InputStream>(_filenames);
    ostream* out = _streams.get<ostream>(_outputFile);
    if (_streams.cinReferences() > 1)
        throw runtime_error("stdin listed more than once!");

    typedef function<void(string&, Vcf::Entry&)> VcfExtractor;
    typedef TypedStream<Vcf::Entry, VcfExtractor> VcfReader;

    typedef function<void (std::string&, Bed&)> BedExtractor;
    typedef TypedStream<Bed, BedExtractor> BedReader;

    typedef OutputWriter<Vcf::Entry> Writer;

    BedExtractor bedEx = bind(&Bed::parseLine, _1, _2, -1); // -1 => parse all fields, not just first 3
    BedReader bedReader(bedEx, *inputStreams[0]);

    Vcf::Header header = Vcf::Header::fromStream(*inputStreams[1]);
    VcfExtractor vcfEx = bind(&Vcf::Entry::parseLine, &header, _1, _2);
    VcfReader vcfReader(vcfEx, *inputStreams[1]);

    Bed bed;
    Vcf::Entry vcf;

    Writer writer(*out);
    Catcher<Writer> c(writer);
    Intersect<VcfReader,BedReader,Catcher<Writer> > intersector(vcfReader, bedReader, c);
    *out << header;
    intersector.execute();
}
