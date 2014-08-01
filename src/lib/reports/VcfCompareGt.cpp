#include "VcfCompareGt.hpp"

#include "common/Integer.hpp"
#include "fileformats/vcf/CustomValue.hpp"
#include "fileformats/vcf/EntryMerger.hpp"
#include "io/StreamJoin.hpp"

#include <boost/filesystem.hpp>

#include <fstream>
#include <memory>
#include <utility>

namespace bfs = boost::filesystem;

VcfCompareGt::VcfCompareGt(
        std::vector<std::string> const& fileNames,
        std::vector<std::string> const& sampleNames,
        std::ostream& out,
        std::string const& outputDir,
        Vcf::Header const& mergedHeader
        )
    : nStreams_(fileNames.size())
    , fileNames_(fileNames)
    , sampleNames_(sampleNames)
    , counts_(sampleNames.size())
    , reportOut_(out)
    , outputDir_(outputDir)
    , mergedHeader_(mergedHeader)
    , mergeStrategy_(&mergedHeader_)
    , mergedOut_(streams_.get<std::ostream>(outputDir + "/all.vcf"))
{
    mergeStrategy_.mergeSamples(true);

    Vcf::CustomType xsec("XSEC", Vcf::CustomType::FIXED_SIZE, nStreams_,
        Vcf::CustomType::INTEGER, "Source file indicator vector");

    mergedHeader_.addInfoType(xsec);
    infoType_ = mergedHeader_.infoType("XSEC");

    *mergedOut_ << mergedHeader_;
}

void VcfCompareGt::flush() {
}

void VcfCompareGt::writeMergedOutput(
        std::set<size_t> const& fileIndices,
        std::map<size_t, Vcf::Entry const*> const& which)
{
    std::vector<Vcf::CustomValue::ValueType> fileIndicators(nStreams_, int64_t(0));
    for (auto i = fileIndices.begin(); i != fileIndices.end(); ++i) {
        fileIndicators[*i] = int64_t(1);
    }

    Vcf::Entry mergedEntry;
    if (which.size() > 1) {
        std::vector<Vcf::Entry> entries;
        for (auto i = which.begin(); i != which.end(); ++i) {
            entries.push_back(*i->second);
        }

        Vcf::EntryMerger merger(
              mergeStrategy_
            , &mergedHeader_
            , entries.data()
            , entries.data() + entries.size()
            );

        mergedEntry = Vcf::Entry(std::move(merger));
    }
    else {
        mergedEntry = *which.begin()->second;
    }

    Vcf::CustomValue xsecInfo(infoType_);
    xsecInfo.setRaw(fileIndicators);
    mergedEntry.setInfo(infoType_->id(), xsecInfo);
    *mergedOut_ << mergedEntry << "\n";
}


void VcfCompareGt::operator()(
        size_t sampleIdx,
        std::string const& sequence,
        Vcf::RawVariant::Vector const& vars,
        std::map<size_t, Vcf::Entry const*> const& which
        )
{
    if (vars.empty() || which.empty()) {
        return;
    }

    std::set<size_t> fileIndices;
    for (auto i = which.begin(); i != which.end(); ++i) {
        fileIndices.insert(i->first);
    }
    ++counts_[sampleIdx][fileIndices];

    writeMergedOutput(fileIndices, which);

    auto& out = getOutputFile(sampleIdx, fileIndices);

    for (auto i = which.begin(); i != which.end(); ++i) {
        if (which.size() > 1) {
            out << "#" << fileNames_[i->first] << "\n";;
        }
        auto const& entry = *i->second;
        entry.allButSamplesToStream(out);
        out << "\t";
        entry.sampleData().formatToStream(out);
        out << "\t";
        auto actualSampleIdx = entry.header().sampleIndex(sampleNames_[sampleIdx]);
        entry.sampleData().sampleToStream(out, actualSampleIdx);
        out << "\n";
    }
    if (which.size() > 1) {
        out << "--\n";
    }
}

void VcfCompareGt::finalize() {
    std::vector<std::vector<size_t>> perSampleCounts;

    for (auto i = fileNames_.rbegin(); i != fileNames_.rend(); ++i) {
        reportOut_ << *i << "\t";
    }


    for (size_t sampleIdx = 0; sampleIdx < sampleNames_.size(); ++sampleIdx) {
        perSampleCounts.push_back(orderedCountsForSample(sampleIdx));
        if (sampleIdx > 0) {
            reportOut_ << "\t";
        }

        reportOut_ << sampleNames_[sampleIdx];
    }
    reportOut_ << "\n";

    size_t maxN = (1 << nStreams_) - 1;
    for (size_t i = 0; i < maxN; ++i) {
        auto binstr = trimmedBinaryString(i + 1);
        reportOut_ << streamJoin(binstr).delimiter("\t");
        for (size_t sampleIdx = 0; sampleIdx < sampleNames_.size(); ++sampleIdx) {
            reportOut_ << "\t" << perSampleCounts[sampleIdx][i];
        }
        reportOut_ << "\n";
    }
}

std::vector<size_t> VcfCompareGt::orderedCountsForSample(size_t sampleIdx) const {
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

std::string VcfCompareGt::trimmedBinaryString(size_t x) const {
    std::string binstr = integerToBinary(x);
    return binstr.substr(binstr.size() - nStreams_);
}

std::string VcfCompareGt::makeFilePath(size_t sampleIdx, std::set<size_t> const& which) const {
    size_t x(0);
    for (auto i = which.begin(); i != which.end(); ++i) {
        x |= 1 << *i;
    }
    bfs::path rv(outputDir_);
    rv /= sampleNames_[sampleIdx] + "_" + trimmedBinaryString(x) + ".txt";
    return rv.string();
}

std::ostream& VcfCompareGt::getOutputFile(size_t sampleIdx, std::set<size_t> const& which) {
    auto path = makeFilePath(sampleIdx, which);
    return *streams_.get<std::ostream>(path);
}

