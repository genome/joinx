#include "VcfFilterHomopolymersCommand.hpp"

#include "common/Sequence.hpp"
#include "fileformats/BedReader.hpp"
#include "fileformats/VcfReader.hpp"
#include "fileformats/vcf/RawVariant.hpp"
#include "processors/IntersectFull.hpp"

#include <boost/program_options.hpp>

#include <iostream>
#include <vector>

namespace po = boost::program_options;

namespace {
    bool allBasesMatch(char a, Vcf::RawVariant const& var) {
        return var.ref.find_first_not_of(a) == std::string::npos &&
            var.alt.find_first_not_of(a) == std::string::npos;
    }

    struct HomopolymerFilter {
        HomopolymerFilter(std::ostream& os, std::size_t maxLength, Vcf::CustomType const* infoType)
            : os_(os)
            , maxLength_(maxLength)
            , infoType_(infoType)
        {
        }

        bool wantMissA() const { return false; }
        bool wantMissB() const { return true; }

        template<typename T1, typename T2>
        bool hit(T1 const& a, T2& b) {
            Vcf::RawVariant::Vector rawvs = Vcf::RawVariant::processEntry(b);

            std::cerr << "HIT: " << a << "\t" << b << "\n";
            char homopolymerBase = a.extraFields()[0][0];


            std::vector<Vcf::CustomValue::ValueType> infoValues(rawvs.size());

            for (std::size_t i = 0; i != rawvs.size(); ++i) {
                auto const& var = rawvs[i];

                if (var.ref.size() != var.alt.size() && allBasesMatch(homopolymerBase, var)) {
                    // do something
                    std::cerr << "FILTER: " << var.alt << "\n";
                    infoValues[i] = int64_t(1);
                }
                else {
                    infoValues[i] = int64_t(0);
                }
            }
            Vcf::CustomValue info(infoType_);
            info.setRaw(infoValues);
            b.setInfo(info.type().id(), info);

            os_ << "HIT: " << b << "\n";


            return true;
        }

        void missA(Bed& x) {}

        void missB(Vcf::Entry const& x) {
            os_ << "MISS: " << x << "\n";
        }

        std::ostream& os_;
        std::size_t maxLength_;
        Vcf::CustomType const* infoType_;
    };
}


VcfFilterHomopolymersCommand::VcfFilterHomopolymersCommand() {
}

void VcfFilterHomopolymersCommand::configureOptions() {
    _opts.add_options()
        ("bed-file,b",
            po::value<std::string>(&homopolymerBedFile_),
            "input homopolymer bed file (created with find-homopolymers command)")

        ("vcf-file,v",
            po::value<std::string>(&vcfFile_),
            "input vcf file to add filters to")

        ("max-length,m",
            po::value<size_t>(&maxLength_)->default_value(3),
            "maximum homopolymer length to filter")

        ("info-field-name,n",
            po::value<std::string>(&infoFieldName_)->default_value("HOMP_FILTER"),
            "name of per-allele info field to use as a filter")

        ("output-file,o",
            po::value<std::string>(&outputFile_)->default_value("-"),
            "output bed file (- for stdout)")
        ;

    _posOpts.add("sequences", -1);
}

void VcfFilterHomopolymersCommand::exec() {
    auto bedStream = _streams.openForReading(homopolymerBedFile_);
    auto vcfStream = _streams.openForReading(vcfFile_);
    auto bedReader = openBed(*bedStream, 1);
    auto vcfReader = openVcf(*vcfStream);

    std::ostream* outStream = _streams.get<std::ostream>(outputFile_);

    Vcf::CustomType infoType(infoFieldName_, Vcf::CustomType::PER_ALLELE, 0,
                        Vcf::CustomType::INTEGER, "short homopolymer indel");
    Vcf::Header newHeader = vcfReader->header();
    newHeader.addInfoType(infoType);
    *outStream << newHeader;

    HomopolymerFilter filter(*outStream, maxLength_, newHeader.infoType(infoFieldName_));
    bool adjacentInsertions = true;
    auto intersector = makeFullIntersector(*bedReader, *vcfReader, filter, adjacentInsertions);
    intersector.execute();
}
