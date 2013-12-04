#include "VcfCompareGt.hpp"
#include "io/StreamJoin.hpp"

#include <boost/bind.hpp>

namespace {
    bool lastRefPosLess(Vcf::RawVariant const& x, Vcf::RawVariant const& y) {
        return x.lastRefPos() < y.lastRefPos();
    }
}

VcfCompareGt::VcfCompareGt(std::vector<std::string> const& sampleNames, std::ostream& out)
    : sampleNames_(sampleNames)
    , counts_(sampleNames.size())
    , out_(out)
{
}

void VcfCompareGt::operator()(
        size_t sampleIdx,
        std::string const& sequence,
        Vcf::RawVariant::Vector const& vars,
        std::vector<size_t> which
        )
{
    if (vars.empty() || which.empty()) {
        return;
    }

    auto i = vars.begin();
    out_ << sampleNames_[sampleIdx]
        << '\t' << sequence
        << '\t'
        ;

    for (; i != vars.end(); ++i) {
        out_ << "(" << i->pos << " " << i->ref << "->" << i->alt << ")";
    }
    out_ << "\t{" << streamJoin(which).delimiter(",") << '}';
    out_ << '\n';

    std::sort(which.begin(), which.end());
    std::stringstream ss;
    ss << streamJoin(which).delimiter(",");
    ++counts_[sampleIdx][ss.str()];
}

void VcfCompareGt::finalize() {
    out_ << "--\n";
    for (size_t sampleIdx = 0; sampleIdx < sampleNames_.size(); ++sampleIdx) {
        out_ << sampleNames_[sampleIdx];
        auto const& cd = counts_[sampleIdx];
        for (auto x = cd.begin(); x != cd.end(); ++x) {
            out_ << "\t" << x->first << ":" << x->second;
        }
        out_ << "\n";
    }
}
