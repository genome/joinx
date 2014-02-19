#include "ConsensusFilter.hpp"
#include "Header.hpp"
#include "Entry.hpp"

#include <stdexcept>

using namespace std;

BEGIN_NAMESPACE(Vcf)

ConsensusFilter::ConsensusFilter(double percent, std::string const& filterName, Header const* hdr)
    : _percent(percent)
    , _filterName(filterName)
    , _header(hdr)
{
}

void ConsensusFilter::apply(Entry& entry, std::vector<size_t> const* counts) const {
    SampleData& sdata = entry.sampleData();
    auto sourceCounts = _header->sampleSourceCounts();
    if (_percent > 0.0 && !_filterName.empty()) {
        for (auto i = sdata.begin(); i != sdata.end(); ++i) {
            auto const& sampleName = entry.header().sampleNames()[i->first];
            size_t mergedIndex = _header->sampleIndex(sampleName);

            if (mergedIndex >= sourceCounts.size())
                throw runtime_error("Couldn't get source count.");

            size_t total = sourceCounts[mergedIndex];
            size_t actual = 0;
            if (counts)
                actual = (*counts)[mergedIndex];
            else
                actual = i->second != 0;

            double pct = actual/double(total);
            if (pct < _percent) {
                sdata.addFilter(i->first, _filterName);
                entry.addFilter(_filterName);
            }
        }
    }
}


END_NAMESPACE(Vcf)
