#include "AlleleMerger.hpp"

#include <boost/algorithm/cxx11/all_of.hpp>
#include <boost/bind.hpp>

#include "Entry.hpp"
#include "RawVariant.hpp"

using namespace std;
namespace ba = boost::algorithm;

BEGIN_NAMESPACE(Vcf)

AlleleMerger::AlleleMerger(Entry const* beg, Entry const* end)
    : _alleleIdx(0)
    , _merged(false)
{
    init(beg, end);
}

AlleleMerger::AlleleMerger(std::vector<Entry> const& ents)
    : _alleleIdx(0)
    , _merged(false)
{
    init(&*ents.begin(), &*ents.end());
}

void AlleleMerger::init(Entry const* beg, Entry const* end) {
    // make sure range is non-trivial and all on the same chromosome
    if (end-beg < 2 || !ba::all_of(beg, end, boost::bind(&Entry::chromEq, beg->chrom(), _1))) {
        return;
    }

    _ref = buildRef(beg, end);
    if (_ref.empty())
        return;

    _merged = true;
    _newAltIndices.resize(end-beg);

    int64_t start = min_element(beg, end, &Entry::posLess)->pos();
    size_t inputAlts(0);
    for (auto e = beg; e != end; ++e) {
        inputAlts += e->alt().size();
        auto rawVariants = RawVariant::processEntry(*e);
        for (auto alt = rawVariants.begin(); alt != rawVariants.end(); ++alt) {
            std::string var = _ref;
            assert(alt->pos >= start);
            size_t varStart = alt->pos - start;
            size_t refLen = alt->ref.size();
            var.replace(varStart, refLen, alt->alt);
            _newAltIndices[e - beg].push_back(addAllele(var));
        }
    }

    _mergedAlt.resize(_alleleMap.size());
    for (auto i = _alleleMap.begin(); i != _alleleMap.end(); ++i)
        _mergedAlt[i->second] = i->first;
}

string AlleleMerger::buildRef(Entry const* beg, Entry const* end) {
    // beg -> end should be sorted by start position
    string ref = beg->ref();
    uint64_t lastPos = beg->pos() + ref.size();

    for (Entry const* e = beg+1; e != end; ++e) {
        if (lastPos < e->pos())
            return "";

        if (e->pos() + e->ref().size() <= lastPos) {
            continue;
        }

        ptrdiff_t off = lastPos - e->pos();

        ref += e->ref().data() + off;
        lastPos += e->ref().size() - off;
    }

    return ref;
}

uint32_t AlleleMerger::addAllele(std::string const& v) {
    auto inserted = _alleleMap.insert(make_pair(v, _alleleIdx));
    if (inserted.second)
        ++_alleleIdx;
    return inserted.first->second;
}

END_NAMESPACE(Vcf)
