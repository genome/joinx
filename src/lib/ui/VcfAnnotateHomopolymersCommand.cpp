#include "VcfAnnotateHomopolymersCommand.hpp"

#include "common/Sequence.hpp"
#include "fileformats/BedReader.hpp"
#include "fileformats/VcfReader.hpp"
#include "fileformats/vcf/RawVariant.hpp"
#include "processors/IntersectFull.hpp"

#include <boost/program_options.hpp>

#include <cstdlib>
#include <iostream>
#include <vector>

namespace po = boost::program_options;

namespace {
    //Ensure that all bases in the variant match the passed base (i.e. the variant is a homopolymer)
    //TODO Add unit tests for this
    bool isSimpleIndel(Vcf::RawVariant const& var, size_t const& maxLength) {
        return var.ref.size() != var.alt.size() &&
            (var.ref.size() == 0 || var.alt.size() == 0) &&
            (var.ref.size() + var.alt.size()) <= maxLength;
    }

    bool allBasesMatch(char a, Vcf::RawVariant const& var) {
        return var.ref.find_first_not_of(a) == std::string::npos &&
            var.alt.find_first_not_of(a) == std::string::npos;
    }

    struct HomopolymerAnnotator {
        HomopolymerAnnotator(std::ostream& os, size_t maxLength, Vcf::CustomType const* infoType)
            : os_(os)
            , maxLength_(maxLength)
            , infoType_(infoType)
        {
        }

        ~HomopolymerAnnotator() {
            flush();
        }

        bool wantMissA() const { return false; }
        bool wantMissB() const { return true; }

        void reset(Vcf::Entry const& b) {
            lastEntry_.reset(new Vcf::Entry(b));
            infoValues_.swap(
                std::vector<Vcf::CustomValue::ValueType>(b.alt().size(), int64_t(0))
                );
        }

        template<typename T1, typename T2>
        bool hit(T1 const& a, T2& b) {
            Vcf::RawVariant::Vector rawvs = Vcf::RawVariant::processEntry(b);
            char homopolymerBase = a.extraFields()[0][0];

            if (lastEntry_) {
                if (lastEntry_->toString() != b.toString()) {
                    flush();
                    reset(b);
                }
            }
            else {
                reset(b);
            }

            for (std::size_t i = 0; i != rawvs.size(); ++i) {
                auto const& var = rawvs[i];

                if (isSimpleIndel(var, maxLength_) &&
                        allBasesMatch(homopolymerBase, var) &&
                        (var.pos - 1) >= a.start() &&
                        (var.pos - 1) <= a.stop()) {
                    infoValues_[i] = int64_t(boost::get<int64_t>(infoValues_[i]) | 1);
                }
                else {
                }
            }

            return true;
        }

        void flush() {
            if (lastEntry_) {
                Vcf::CustomValue info(infoType_);
                info.setRaw(infoValues_);
                lastEntry_->setInfo(info.type().id(), info);
                os_ << *lastEntry_ << "\n";
                lastEntry_.reset();
            }
        }

        void missA(Bed& x) {}

        void missB(Vcf::Entry const& x) {
            flush();
            //os_ << "MISS: " << x << "\n";
            os_ << x << "\n";
        }

        std::ostream& os_;
        size_t maxLength_;
        Vcf::CustomType const* infoType_;

        std::unique_ptr<Vcf::Entry> lastEntry_;
        std::vector<Vcf::CustomValue::ValueType> infoValues_;
    };
}


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

    HomopolymerAnnotator annotator(*outStream, maxLength_, newHeader.infoType(infoFieldName_));
    bool adjacentInsertions = true;
    auto intersector = makeFullIntersector(*bedReader, *vcfReader, annotator, adjacentInsertions);
    intersector.execute();
}
