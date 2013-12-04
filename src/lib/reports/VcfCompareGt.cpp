#include "VcfCompareGt.hpp"
#include "common/Integer.hpp"
#include "io/StreamJoin.hpp"

#include <boost/bind.hpp>
#include <boost/filesystem.hpp>
#include <boost/functional/hash.hpp>

namespace bfs = boost::filesystem;

class VcfCompareGt::Impl {
public:
    Impl(
            std::vector<std::string> const& fileNames,
            std::vector<std::string> const& sampleNames,
            std::ostream& out)
        : nStreams_(fileNames.size())
        , fileNames_(fileNames)
        , sampleNames_(sampleNames)
        , counts_(sampleNames.size())
        , out_(out)
    {
    }

    void operator()(
            size_t sampleIdx,
            std::string const& sequence,
            Vcf::RawVariant::Vector const& vars,
            std::set<size_t> const& which
            )
    {
        if (vars.empty() || which.empty()) {
            return;
        }

        ++counts_[sampleIdx][which];
    }

    void finalize() {
        std::vector<std::vector<size_t>> perSampleCounts;

        for (auto i = fileNames_.begin(); i != fileNames_.end(); ++i) {
            out_ << *i << "\t";
        }


        for (size_t sampleIdx = 0; sampleIdx < sampleNames_.size(); ++sampleIdx) {
            perSampleCounts.push_back(orderedCountsForSample(sampleIdx));
            if (sampleIdx > 0) {
                out_ << "\t";
            }

            out_ << sampleNames_[sampleIdx];
        }
        out_ << "\n";

        size_t maxN = (1 << nStreams_) - 1;
        for (size_t i = 0; i < maxN; ++i) {
            auto binstr = integerToBinary(i + 1);
            binstr = binstr.substr(binstr.size() - nStreams_);
            out_ << streamJoin(binstr).delimiter("\t");
            for (size_t sampleIdx = 0; sampleIdx < sampleNames_.size(); ++sampleIdx) {
                out_ << "\t" << perSampleCounts[sampleIdx][i];
            }
            out_ << "\n";
        }
    }

    std::vector<size_t> orderedCountsForSample(size_t sampleIdx) const {
        std::vector<size_t> rv((1 << nStreams_) - 1, 0);
        auto const& x = counts_[sampleIdx];
        for (auto i = x.begin(); i != x.end(); ++i) {
            size_t word = 0;
            auto& which = i->first;
            for (auto bit = which.begin(); bit != which.end(); ++bit) {
                word |= 1 << *bit;
            }
            rv[word - 1] = i->second;
        }
        return rv;
    }

private:
    size_t nStreams_;
    std::vector<std::string> const& fileNames_;
    std::vector<std::string> const& sampleNames_;
    std::vector<
        boost::unordered_map<std::set<size_t>, size_t>
        > counts_;
    std::ostream& out_;
};

VcfCompareGt::VcfCompareGt(
        std::vector<std::string> const& fileNames,
        std::vector<std::string> const& sampleNames,
        std::ostream& out)
    : impl_(new Impl(fileNames, sampleNames, out))
{
}

VcfCompareGt::~VcfCompareGt() {
}

void VcfCompareGt::operator()(
        size_t sampleIdx,
        std::string const& sequence,
        Vcf::RawVariant::Vector const& vars,
        std::set<size_t> const& which
        )
{
    (*impl_)(sampleIdx, sequence, vars, which);
}

void VcfCompareGt::finalize() {
    impl_->finalize();
}

std::vector<size_t> VcfCompareGt::orderedCountsForSample(size_t sampleIdx) const {
    return impl_->orderedCountsForSample(sampleIdx);
}
