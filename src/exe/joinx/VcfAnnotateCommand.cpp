#include "VcfAnnotateCommand.hpp"

#include "common/Tokenizer.hpp"
#include "fileformats/Bed.hpp"
#include "fileformats/InputStream.hpp"
#include "fileformats/OutputWriter.hpp"
#include "fileformats/TypedStream.hpp"
#include "fileformats/vcf/Compare.hpp"
#include "fileformats/vcf/CustomType.hpp"
#include "fileformats/vcf/CustomValue.hpp"
#include "fileformats/vcf/Entry.hpp"
#include "fileformats/vcf/Header.hpp"
#include "processors/IntersectSimple.hpp"

#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <functional>
#include <iterator>
#include <stdexcept>

namespace po = boost::program_options;
using boost::format;
using namespace std;
using namespace placeholders;

CommandBase::ptr VcfAnnotateCommand::create(int argc, char** argv) {
    boost::shared_ptr<VcfAnnotateCommand> app(new VcfAnnotateCommand);
    app->parseArguments(argc, argv);
    return app;
}

VcfAnnotateCommand::VcfAnnotateCommand()
    : _outputFile("-")
    , _copyIdents(true)
    , _copyInfo(true)
{
}

void VcfAnnotateCommand::parseArguments(int argc, char** argv) {
    po::options_description opts("Available Options");
    opts.add_options()
        ("help,h", "this message")
        ("input-file,i", po::value<string>(&_vcfFile), "input file")
        ("annotation-file,a", po::value<string>(&_annoFile), "VCF file containing annotation data")
        ("output-file,o", po::value<string>(&_outputFile), "output file (empty or - means stdout, which is the default)")
        ("info-fields,I", po::value<vector<string>>(&_infoFields), "info fields to use for annotation (default: all)")
        ("no-info", "do not copy info fields")
        ("no-identifiers", "do not copy identifiers from the annotation file")
        ;
    po::positional_options_description posOpts;
    posOpts.add("input-file", 1);
    posOpts.add("annotation-file", 1);

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

    if (!vm.count("input-file"))
        throw runtime_error("Required argument 'input-file' missing");

    if (!vm.count("annotation-file"))
        throw runtime_error("Required argument 'annotation-file' missing");

    if (vm.count("no-identifiers"))
        _copyIdents = false;

    if (vm.count("no-info")) {
        _copyInfo = false;
        _infoFields.clear();
    }
}

void VcfAnnotateCommand::postProcessArguments(Vcf::Header& header, Vcf::Header const& annoHeader) {
    if (_copyInfo && _infoFields.empty()) {
        auto const& annoInfo = annoHeader.infoTypes();
        for (auto iter = annoInfo.begin(); iter != annoInfo.end(); ++iter)
            _infoFields.push_back(iter->first);
    }

    for (auto iter = _infoFields.begin(); iter != _infoFields.end(); ++iter) {
        vector<string> tokens;
        Tokenizer<string>::split(*iter, "=,", back_inserter(tokens));
        if (tokens.empty()) {
            throw runtime_error("Invalid value for info field (null)");
        }

        Vcf::CustomType const* oldType = annoHeader.infoType(tokens[0]);
        if (!oldType) {
            throw runtime_error(str(format(
                "Unknown info field '%1%' for annotation file"
                ) %tokens[0]));
        }

        string id = oldType->id();
        if (tokens.size() > 1) {
            id = tokens[1];
        }

        InfoTranslation itxl;

        Vcf::CustomType::NumberType numberType = oldType->numberType();
        size_t number = oldType->number();

        if (tokens.size() > 2) {
            if (tokens[2] != "per-alt") {
                throw runtime_error(str(format(
                    "Invalid syntax for info field specification: '%1%'") %*iter));
            }
            itxl.singleToPerAlt = true;
            numberType = Vcf::CustomType::PER_ALLELE;
            number = 0;
        } else {
            itxl.singleToPerAlt = false;
        }

        Vcf::CustomType newType(
            id,
            numberType,
            number,
            oldType->type(),
            oldType->description()
            );
        header.addInfoType(newType);

        itxl.newType = header.infoType(id);
        _infoMap[oldType->id()] = itxl;
    } 
}


void VcfAnnotateCommand::exec() {
    vector<string> filenames;
    filenames.push_back(_vcfFile);
    filenames.push_back(_annoFile);
    vector<InputStream::ptr> inputStreams = _streams.wrap<istream, InputStream>(filenames);
    ostream* out = _streams.get<ostream>(_outputFile);
    if (_streams.cinReferences() > 1)
        throw runtime_error("stdin listed more than once!");

    typedef function<void(const Vcf::Header*, string&, Vcf::Entry&)> VcfExtractor;
    typedef TypedStream<Vcf::Entry, VcfExtractor> VcfReader;
    typedef OutputWriter<Vcf::Entry> Writer;
    typedef SimpleVcfAnnotator<Writer> AnnoType;

    VcfExtractor vcfEx = bind(&Vcf::Entry::parseLine, _1, _2, _3);
    VcfReader vcfReader(vcfEx, *inputStreams[0]);
    VcfReader annoReader(vcfEx, *inputStreams[1]);

    Vcf::Header& annoHeader = annoReader.header();
    Vcf::Header& header = vcfReader.header();

    header.add(str(format("##annotation=%s") %_annoFile));

    postProcessArguments(header, annoHeader);

    Writer writer(*out);
    AnnoType c(writer, _copyIdents, _infoMap, header);
    IntersectSimple<VcfReader,VcfReader,AnnoType> intersector(vcfReader, annoReader, c);
    *out << vcfReader.header();
    intersector.execute();
}
