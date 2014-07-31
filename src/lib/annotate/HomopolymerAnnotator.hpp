#pragma once

#include "fileformats/Bed.hpp"
#include "fileformats/vcf/CustomType.hpp"
#include "fileformats/vcf/CustomValue.hpp"
#include "fileformats/vcf/Entry.hpp"
#include "fileformats/vcf/RawVariant.hpp"

#include <cstddef>
#include <ostream>
#include <vector>
#include <string>

class HomopolymerAnnotator {
public:
    HomopolymerAnnotator(std::ostream& os, size_t maxLength, Vcf::CustomType const* infoType);
    ~HomopolymerAnnotator();

    bool wantMissA() const { return false; }
    bool wantMissB() const { return true; }

    void missA(Bed& x);
    void missB(Vcf::Entry const& x);

    void reset(Vcf::Entry const& b);
    void flush();

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



private:
    std::ostream& os_;
    size_t maxLength_;
    Vcf::CustomType const* infoType_;

    std::unique_ptr<Vcf::Entry> lastEntry_;
    std::vector<Vcf::CustomValue::ValueType> infoValues_;
};

