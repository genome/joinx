#include "VcfAnnotateCommand.hpp"

#include "common/Tokenizer.hpp"
#include "fileformats/VcfReader.hpp"
#include "fileformats/Bed.hpp"
#include "fileformats/InputStream.hpp"
#include "fileformats/OutputWriter.hpp"
#include "fileformats/vcf/Compare.hpp"
#include "fileformats/vcf/CustomType.hpp"
#include "fileformats/vcf/CustomValue.hpp"
#include "fileformats/vcf/Entry.hpp"
#include "fileformats/vcf/Header.hpp"
#include "processors/IntersectSimple.hpp"

#include <boost/bind.hpp>
#include <boost/format.hpp>
#include <boost/function.hpp>
#include <boost/program_options.hpp>

#include <iterator>
#include <stdexcept>

namespace po = boost::program_options;
using boost::format;
using namespace std;

VcfAnnotateCommand::VcfAnnotateCommand()
    : _outputFile("-")
{
}

void VcfAnnotateCommand::configureOptions() {
    _opts.add_options()
        ("input-file,i",
            po::value<string>(&_vcfFile)->required(),
            "input file (required)")

        ("annotation-file,a",
            po::value<string>(&_annoFile)->required(),
            "VCF file containing annotation data")

        ("output-file,o",
            po::value<string>(&_outputFile)->default_value("-"),
            "output file (empty or - means stdout, which is the default)")

        ("info-fields,I",
            po::value<vector<string>>(&_infoFields),
            "info fields to use for annotation (default: all)")

        ("no-info",
            po::bool_switch(&_noInfo),
            "do not copy info fields")

        ("no-identifiers",
            po::bool_switch(&_noIdents),
            "do not copy identifiers from the annotation file")
        ;

    _posOpts.add("input-file", 1);
    _posOpts.add("annotation-file", 1);
}

void VcfAnnotateCommand::postProcessArguments(Vcf::Header& header, Vcf::Header const& annoHeader) {
    if (_noInfo) {
        _infoFields.clear();
    }
    else if (_infoFields.empty()) {
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
    typedef OutputWriter<Vcf::Entry> Writer;
    typedef SimpleVcfAnnotator<Writer> AnnoType;

    auto vcfIn = _streams.openForReading(_vcfFile);
    auto annoIn = _streams.openForReading(_annoFile);

    ostream* out = _streams.get<ostream>(_outputFile);
    if (_streams.cinReferences() > 1)
        throw runtime_error("stdin listed more than once!");
    auto vcfReader = openVcf(*vcfIn);
    auto annoReader = openVcf(*annoIn);

    Vcf::Header& annoHeader = annoReader->header();
    Vcf::Header& header = vcfReader->header();
    header.add(str(format("##annotation=%s") % _annoFile));

    postProcessArguments(header, annoHeader);

    Writer writer(*out);
    AnnoType c(writer, !_noIdents, _infoMap, header);
    auto intersector = makeSimpleIntersector(*vcfReader, *annoReader, c);
    *out << vcfReader->header();
    intersector.execute();
}
