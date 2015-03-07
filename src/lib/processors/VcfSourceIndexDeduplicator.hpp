#pragma once

#include "fileformats/vcf/Entry.hpp"
#include "fileformats/vcf/Header.hpp"

#include <boost/unordered_set.hpp>

#include <memory>
#include <vector>

template<typename PassOutputFunc, typename FailOutputFunc>
class VcfSourceIndexDeduplicator {
public:
    typedef std::unique_ptr<Vcf::Entry> ValuePtr;
    typedef std::vector<ValuePtr> ValuePtrVector;

    VcfSourceIndexDeduplicator(PassOutputFunc& passOut, FailOutputFunc& failOut, bool active)
        : passOut_(passOut)
        , failOut_(failOut)
        , active_(active)
    {}

    void operator()(ValuePtrVector&& entries) {
        boost::unordered_set<std::size_t> seen;
        ValuePtrVector pass;
        ValuePtrVector fail;
        for (auto i = entries.begin(); i != entries.end(); ++i) {
            auto const& e = **i;
            std::size_t sourceIdx = e.header().sourceIndex();
            auto inserted = seen.insert(sourceIdx);
            if (!active_ || inserted.second) {
                // first time to see this source index
                pass.push_back(std::move(*i));
            }
            else {
                fail.push_back(std::move(*i));
            }
        }
        passOut_(std::move(pass));
        failOut_(std::move(fail));
    }

private:
    PassOutputFunc& passOut_;
    FailOutputFunc& failOut_;
    bool active_;
};

template<typename PassOutputFunc, typename FailOutputFunc>
VcfSourceIndexDeduplicator<PassOutputFunc, FailOutputFunc>
makeVcfSourceIndexDeduplicator(
          PassOutputFunc& passOut
        , FailOutputFunc failOut
        , bool active
        )
{
    return VcfSourceIndexDeduplicator<PassOutputFunc, FailOutputFunc>(passOut, failOut, active);
}
