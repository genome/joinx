#pragma once

#include "common/CoordinateView.hpp"

#include <boost/config.hpp>
#include <boost/graph/adjacency_matrix.hpp>
#include <boost/graph/connected_components.hpp>
#include <boost/unordered_set.hpp>

#include <memory>
#include <vector>

// FIXME: this doesn't belong here! we'll want to generalize the
// region extraction concept and put this vcf-specific one with vcf
// stuff
#include "fileformats/vcf/RawVariant.hpp"
#include "fileformats/vcf/Entry.hpp"

struct VcfRegionExtractor {
    typedef boost::unordered_set<Region> ReturnType;

    ReturnType operator()(Vcf::Entry const& entry) const {
        auto rawvs = Vcf::RawVariant::processEntry(entry);
        ReturnType rv;
        for (auto i = rawvs.begin(); i != rawvs.end(); ++i) {
            // if we set the region begin to the region end then
            // we would be reporting only the start position.
            // that is a useful option sometimes.
            rv.insert(i->region());
        }
        return rv;
    }
};
// END FIXME


template<
          typename ValueType
        , typename OutputFunc
        , typename RegionExtractor = VcfRegionExtractor // FIXME: get rid of default
        >
class GroupBySharedRegions {
public:
    typedef std::unique_ptr<ValueType> ValuePtr;
    typedef std::vector<ValuePtr> ValuePtrVector;
    typedef typename RegionExtractor::ReturnType RegionSet;

    GroupBySharedRegions(
              OutputFunc& out
            , RegionExtractor regionExtractor = RegionExtractor()
            )
        : out_(out)
        , regionExtractor_(regionExtractor)
    {}

    template<typename Members, typename Graph>
    void add_component(Members const& xs, Graph& g) {
        using namespace boost;
        for (std::size_t i = 0; i < xs.size(); ++i) {
            for (std::size_t j = i + 1; j < xs.size(); ++j) {
                add_edge(xs[i], xs[j], g);
            }
        }
    }

    void operator()(ValuePtrVector entries) {
        std::vector<RegionSet> regions(entries.size());
        boost::unordered_map<Region, boost::unordered_set<std::size_t>> regionToEntries;
        for (std::size_t i = 0; i < entries.size(); ++i) {
            std::size_t entryIdx = i;
            regions[i] = regionExtractor_(*entries[i]);
            for (auto j = regions[i].begin();j != regions[i].end(); ++j) {
                regionToEntries[*j].insert(entryIdx);
            }
        }

        using namespace boost;
        typedef adjacency_matrix<undirectedS> Graph;
        Graph graph(entries.size());

        for (auto x = regionToEntries.begin(); x != regionToEntries.end(); ++x) {
            std::vector<std::size_t> xs(x->second.begin(), x->second.end());
            add_component(xs, graph);
        }

        std::vector<int> components(entries.size());
        connected_components(graph, &components[0]);

        boost::unordered_map<int, ValuePtrVector> groups;
        for (std::size_t i = 0; i < components.size(); ++i) {
            groups[components[i]].push_back(std::move(entries[i]));
        }

        for (auto i = groups.begin(); i != groups.end(); ++i) {
            out_(std::move(i->second));
        }
    }

private:
    OutputFunc& out_;
    RegionExtractor regionExtractor_;
};

template<
          typename ValueType
        , typename OutputFunc
        , typename RegionExtractor = VcfRegionExtractor // FIXME: get rid of default
        >
GroupBySharedRegions<ValueType, OutputFunc, RegionExtractor>
makeGroupBySharedRegions(
              OutputFunc& out
            , RegionExtractor regionExtractor = RegionExtractor()
            )
{
    return GroupBySharedRegions<ValueType, OutputFunc, RegionExtractor>(
        out, regionExtractor
        );
}
