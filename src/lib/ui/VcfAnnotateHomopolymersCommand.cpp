#include "VcfAnnotateHomopolymersCommand.hpp"

#include "annotate/HomopolymerAnnotator.hpp"
#include "common/Sequence.hpp"
#include "fileformats/BedReader.hpp"
#include "fileformats/OutputWriter.hpp"
#include "fileformats/VcfReader.hpp"
#include "fileformats/vcf/Entry.hpp"
#include "fileformats/vcf/RawVariant.hpp"
#include "processors/IntersectFull.hpp"

#include <boost/program_options.hpp>

#include <cstdlib>
#include <iostream>
#include <vector>

namespace po = boost::program_options;

VcfAnnotateHomopolymersCommand::VcfAnnotateHomopolymersCommand() {
}

void VcfAnnotateHomopolymersCommand::configureOptions() {
    _opts.add_options()
        ("bed-file,b",
            po::value<std::string>(&homopolymerBedFile_),
            "input homopolymer bed file (created with find-homopolymers command and should be padded by 1bp using bedtools slop or other command)")

        ("vcf-file,v",
            po::value<std::string>(&vcfFile_),
            "input vcf file to add annotation to")

        ("max-length,m",
            po::value<size_t>(&maxLength_)->default_value(2),
            "maximum indel length to annotate as in the homopolymer")

        ("info-field-name,n",
            po::value<std::string>(&infoFieldName_)->default_value("HOMP_FILTER"),
            "name of per-allele info field to store the annotation")

        ("output-file,o",
            po::value<std::string>(&outputFile_)->default_value("-"),
            "output bed file (- for stdout)")
        ;

    _posOpts.add("sequences", -1);
}

void VcfAnnotateHomopolymersCommand::exec() {
    auto bedStream = _streams.openForReading(homopolymerBedFile_);
    auto vcfStream = _streams.openForReading(vcfFile_);
    auto bedReader = openBed(*bedStream, 1);
    auto vcfReader = openVcf(*vcfStream);

    std::ostream* outStream = _streams.get<std::ostream>(outputFile_);

    std::stringstream description;
    description << "short (maximum of " << maxLength_ << "bp) homopolymer indel";

    Vcf::CustomType infoType(infoFieldName_, Vcf::CustomType::PER_ALLELE, 0,
                        Vcf::CustomType::INTEGER, description.str());
    Vcf::Header newHeader = vcfReader->header();
    newHeader.addInfoType(infoType);
    *outStream << newHeader;

    OutputWriter<Vcf::Entry> out(*outStream);
    auto annotator = makeHomopolymerAnnotator(out, maxLength_,
            newHeader.infoType(infoFieldName_));

    bool adjacentInsertions = true;
    auto intersector = makeFullIntersector(*bedReader, *vcfReader, annotator, adjacentInsertions);
    intersector.execute();
}
