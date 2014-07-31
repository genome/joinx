#include "HomopolymerAnnotator.hpp"

HomopolymerAnnotator::HomopolymerAnnotator(
        std::ostream& os,
        size_t maxLength,
        Vcf::CustomType const* infoType)
    : os_(os)
    , maxLength_(maxLength)
    , infoType_(infoType)
{
}

HomopolymerAnnotator::~HomopolymerAnnotator() {
    flush();
}

void HomopolymerAnnotator::reset(Vcf::Entry const& b) {
    lastEntry_.reset(new Vcf::Entry(b));
    infoValues_.swap(
        std::vector<Vcf::CustomValue::ValueType>(b.alt().size(), int64_t(0))
        );
}

void HomopolymerAnnotator::flush() {
    if (lastEntry_) {
        Vcf::CustomValue info(infoType_);
        info.setRaw(infoValues_);
        lastEntry_->setInfo(info.type().id(), info);
        os_ << *lastEntry_ << "\n";
        lastEntry_.reset();
    }
}

void HomopolymerAnnotator::missA(Bed& x) {}

void HomopolymerAnnotator::missB(Vcf::Entry const& x) {
    flush();
    os_ << x << "\n";
}
