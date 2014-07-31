#pragma once

#include "fileformats/Bed.hpp"
#include "fileformats/vcf/CustomType.hpp"
#include "fileformats/vcf/CustomValue.hpp"
#include "fileformats/vcf/Entry.hpp"
#include "fileformats/vcf/RawVariant.hpp"

#include <boost/optional.hpp>

#include <cstddef>
#include <ostream>
#include <vector>
#include <string>

template<typename Writer>
class HomopolymerAnnotator {
public:
    HomopolymerAnnotator(Writer& writer, size_t maxLength, Vcf::CustomType const* infoType);
    ~HomopolymerAnnotator();

    bool wantMissA() const { return false; }
    void missA(Bed const&) {};

    bool wantMissB() const { return true; }
    void missB(Vcf::Entry const& x);

    void reset(Vcf::Entry const& b);
    void flush();

    bool hit(Bed const& a, Vcf::Entry& b) {
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



private:
    Writer& writer_;
    size_t maxLength_;
    Vcf::CustomType const* infoType_;

    boost::optional<Vcf::Entry> lastEntry_;
    std::vector<Vcf::CustomValue::ValueType> infoValues_;
};

template<typename Writer>
HomopolymerAnnotator<Writer> makeHomopolymerAnnotator(
    Writer& writer,
    size_t maxLength,
    Vcf::CustomType const* infoType)
{
    return HomopolymerAnnotator<Writer>(writer, maxLength, infoType);
}


template<typename Writer>
HomopolymerAnnotator<Writer>::HomopolymerAnnotator(
        Writer& writer,
        size_t maxLength,
        Vcf::CustomType const* infoType)
    : writer_(writer)
    , maxLength_(maxLength)
    , infoType_(infoType)
{
}

template<typename Writer>
HomopolymerAnnotator<Writer>::~HomopolymerAnnotator() {
    flush();
}

template<typename Writer>
void HomopolymerAnnotator<Writer>::reset(Vcf::Entry const& b) {
    lastEntry_ = b;
    infoValues_.swap(
        std::vector<Vcf::CustomValue::ValueType>(b.alt().size(), int64_t(0))
        );
}

template<typename Writer>
void HomopolymerAnnotator<Writer>::flush() {
    if (lastEntry_) {
        Vcf::CustomValue info(infoType_);
        info.setRaw(infoValues_);
        lastEntry_->setInfo(info.type().id(), info);
        writer_(*lastEntry_);
        lastEntry_.reset();
    }
}

template<typename Writer>
void HomopolymerAnnotator<Writer>::missB(Vcf::Entry const& x) {
    flush();
    writer_(x);
}

