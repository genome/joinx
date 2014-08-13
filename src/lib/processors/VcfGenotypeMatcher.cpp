#include "VcfGenotypeMatcher.hpp"

#include "fileformats/vcf/CustomType.hpp"
#include "fileformats/vcf/CustomValue.hpp"
#include "fileformats/vcf/Header.hpp"
#include "io/StreamJoin.hpp"

#include <boost/format.hpp>

#include <algorithm>
#include <cassert>
#include <iostream>
#include <iterator>
#include <stdexcept>

using namespace Vcf;
using boost::container::flat_set;
using boost::format;


namespace {
    Vcf::CustomType const* getType(Vcf::Header const& header, std::string const& id) {
        Vcf::CustomType const* type = header.formatType(id);
        if (!type) {
            throw std::runtime_error(str(format(
                "Format field %1% not found in vcf header for file %2%"
                ) % id % header.sourceIndex()));
        }
        return type;
    }

    template<typename VecType, typename SetType>
    void setValues(VecType& v, SetType const& s) {
        for (auto i = s.begin(); i != s.end(); ++i) {
            assert(v.size() > *i);
            v[*i] = int64_t{1};
        }
    }

    template<typename Container>
    Container difference_(Container const& s1, Container const& s2) {
        Container rv;
        std::set_difference(s1.begin(), s1.end(), s2.begin(), s2.end(),
            std::inserter(rv, rv.begin()));
        return rv;
    }
}

void GenotypeDictionary::add(Vcf::RawVariant::Vector const& gt, size_t entryIdx) {
    exactMap_[gt].insert(entryIdx);
    for (auto x = gt.begin(); x != gt.end(); ++x) {
        ++partialMap_[*x][entryIdx];
    }
}

auto GenotypeDictionary::allMatches(Vcf::RawVariant const& gt) const -> LocationCounts const* {
    auto iter = partialMap_.find(gt);
    if (iter != partialMap_.end()) {
        return &iter->second;
    }
    return 0;
}

auto GenotypeDictionary::exactMatches(Vcf::RawVariant::Vector const& gt) const -> Locations const* {
    auto iter = exactMap_.find(gt);
    if (iter != exactMap_.end()) {
        return &iter->second;
    }
    return 0;
}

void GenotypeDictionary::clear() {
    partialMap_.clear();
    exactMap_.clear();
}


VcfGenotypeMatcher::VcfGenotypeMatcher(
          std::vector<std::string> const& streamNames
        , std::vector<std::string> const& sampleNames
        , std::string const& exactFieldName
        , std::string const& partialFieldName
        , std::string const& outputDir
        )
    : numFiles_(streamNames.size())
    , numSamples_(sampleNames.size())
    , exactFieldName_(exactFieldName)
    , partialFieldName_(partialFieldName)
    , sampleNames_(sampleNames)
    , streamNames_(streamNames)
    , outputDir_(outputDir)
    , gtDicts_(numSamples_)
    , sampleCounters_(numSamples_)
    , wroteHeader_(numFiles_, false)
{
}

void VcfGenotypeMatcher::collectEntry(size_t entryIdx) {
    Vcf::Entry const& entry = *entries_[entryIdx];
    auto rawvs = RawVariant::processEntry(entry);
    auto const& sampleData = entry.sampleData();

    SampleGenotypes sampleGenotypes(numSamples_);

    for (uint32_t rawSampleIdx = 0; rawSampleIdx < numSamples_; ++rawSampleIdx) {
        uint32_t sampleIdx = entry.header().sampleIndex(sampleNames_[rawSampleIdx]);

        GenotypeCall const& call = sampleData.genotype(sampleIdx);
        if (call == GenotypeCall::Null)
            continue;

        // FIXME: try to copy the RawVariants less
        RawVariant::Vector gtvec;
        for (auto idx = call.indices().begin(); idx != call.indices().end(); ++idx) {
            if (*idx != Vcf::GenotypeIndex::Null && idx->value > 0) {
                auto const& allele = rawvs[idx->value - 1];
                gtvec.push_back(new RawVariant(allele));
            }
        }

        gtvec.sort();
        gtDicts_[rawSampleIdx].add(gtvec, entryIdx);
        sampleGenotypes[rawSampleIdx] = std::move(gtvec);
    }

    entryGenotypes_.push_back(std::move(sampleGenotypes));
}

void VcfGenotypeMatcher::updateCounts() {
    for (size_t rawSampleIdx = 0; rawSampleIdx < numSamples_; ++rawSampleIdx) {
        auto alleleMatches = gtDicts_[rawSampleIdx].copyPartialMatches();
        for (auto al = alleleMatches.begin(); al != alleleMatches.end(); ++al) {
            auto& locationCounts = al->second;
            std::set<size_t> files;
            for (auto lc = locationCounts.begin(); lc != locationCounts.end(); ) {
                size_t entryIdx = lc->first;
                size_t fileIdx = entries_[entryIdx]->header().sourceIndex();
                size_t& count = lc->second;
                if (count > 0) {
                    files.insert(fileIdx);
                    --count;
                    ++lc;
                }
                else {
                    lc = locationCounts.erase(lc);
                }
            }
            ++sampleCounters_[rawSampleIdx][files];
        }
    }
}

void VcfGenotypeMatcher::annotateEntry(size_t entryIdx) {
    Entry& entry = *entries_[entryIdx];
    auto exactType = getType(entry.header(), exactFieldName_);
    auto partialType = getType(entry.header(), partialFieldName_);

    auto const& sampleGenotypes = entryGenotypes_[entryIdx];
    auto& sampleData = entry.sampleData();

    for (uint32_t rawSampleIdx = 0; rawSampleIdx < numSamples_; ++rawSampleIdx) {
        auto const& dict = gtDicts_[rawSampleIdx];
        auto const& genotype = sampleGenotypes[rawSampleIdx];

        flat_set<size_t> partials;

        flat_set<size_t> exactMatches;
        auto const* exactMatchesEntries = dict.exactMatches(sampleGenotypes[rawSampleIdx]);
        // exactMatchesEntries is a set of entry indices; we want to convert
        // such to file indices
        if (exactMatchesEntries) {
            for (auto i = exactMatchesEntries->begin(); i != exactMatchesEntries->end(); ++i) {
                exactMatches.insert(entries_[*i]->header().sourceIndex());
            }
        }

        // FIXME: don't copy vars; make and use a pointer hasher
        boost::unordered_set<RawVariant> seen;
        for (auto ai = genotype.begin(); ai != genotype.end(); ++ai) {
            auto inserted = seen.insert(*ai);
            if (!inserted.second)
                continue; // already processed this allele

            auto const& allele = *ai;

            auto const* xsec = dict.allMatches(allele);
            if (xsec) {
                for (auto j = xsec->begin(); j != xsec->end(); ++j) {
                    size_t whichEntry = j->first;
                    size_t fileIdx = entries_[whichEntry]->header().sourceIndex();
                    partials.insert(fileIdx);
                }
            }
        }

        std::vector<Vcf::CustomValue::ValueType> exactValues(numFiles_, int64_t{0});
        std::vector<Vcf::CustomValue::ValueType> partialValues(numFiles_, int64_t{0});

        partials = difference_(partials, exactMatches);
        setValues(exactValues, exactMatches);

        setValues(partialValues, partials);

        uint32_t sampleIdx = entry.header().sampleIndex(sampleNames_[rawSampleIdx]);
        sampleData.setSampleField(sampleIdx, Vcf::CustomValue(exactType, std::move(exactValues)));
        sampleData.setSampleField(sampleIdx, Vcf::CustomValue(partialType, std::move(partialValues)));
    }
}

void VcfGenotypeMatcher::operator()(EntryList&& entries) {
    entries_.swap(entries);

    for (size_t i = 0; i < entries_.size(); ++i)
        collectEntry(i);

    assert(entryGenotypes_.size() == entries_.size());

    for (size_t i = 0; i < entries_.size(); ++i)
        annotateEntry(i);

    updateCounts();
    writeEntries();
    reset();
}

void VcfGenotypeMatcher::reset() {
    for (auto di = gtDicts_.begin(); di != gtDicts_.end(); ++di)
        di->clear();

    entryGenotypes_.clear();
    entries_.clear();
}

void VcfGenotypeMatcher::writeEntries() const {
    for (auto ei = entries_.begin(); ei != entries_.end(); ++ei) {
        auto const& entry = **ei;
        size_t fileIdx = entry.header().sourceIndex();
        std::ostream& os = getStream(fileIdx);

        assert(fileIdx < wroteHeader_.size());
        if (!wroteHeader_[fileIdx]) {
            os << entry.header();
            wroteHeader_[fileIdx] = true;
        }
        os << entry << "\n";
    }
}

void VcfGenotypeMatcher::reportCounts(std::ostream& os) const {
    for (size_t rawSampleIdx = 0; rawSampleIdx < numSamples_; ++rawSampleIdx) {
        os << "Sample " << sampleNames_[rawSampleIdx] << "\n";
        auto const& counts = sampleCounters_[rawSampleIdx];
        for (auto ci = counts.begin(); ci != counts.end(); ++ci) {
            auto const& fileSet = ci->first;
            size_t count = ci->second;
            os << streamJoin(fileSet) << ": " << count << "\n";
        }
    }
}

std::ostream& VcfGenotypeMatcher::getStream(size_t idx) const {
    // FIXME: real filenames please
    std::stringstream ss;
    ss << outputDir_ << "/";

    if (idx < streamNames_.size() && !streamNames_[idx].empty())
        ss << streamNames_[idx];
    else
        ss << idx;

    ss << ".vcf";

    return* streams_.get<std::ostream>(ss.str());
}
