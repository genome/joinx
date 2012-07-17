#include "AlleleMerger.hpp"

#include "Entry.hpp"

using namespace std;
using namespace std::placeholders;

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
    if (end-beg < 2 || !all_of(beg, end, bind(&Entry::chromEq, beg->chrom(), _1))) {
        return;
    }

    _ref = buildRef(beg, end);
    if (_ref.empty())
        return;

    _merged = true;
    _newGt.resize(end-beg);

    uint64_t start = min_element(beg, end, &Entry::posLess)->pos();
    size_t inputAlts(0);
    for (auto e = beg; e != end; ++e) {
        inputAlts += e->alt().size();
        for (auto alt = e->alt().begin(); alt != e->alt().end(); ++alt) {
            std::string var = _ref;
            int64_t off = e->pos() - start;
            int32_t len = alt->size() - e->ref().size();
            if (len == 0) { // SNV
                for (std::string::size_type i = 0; i < alt->size(); ++i)
                    var[off+i] = (*alt)[i];
                _newGt[e-beg].push_back(addAllele(var));
            } else if (len < 0) {
                var.erase(off+alt->size(), -len);
                _newGt[e-beg].push_back(addAllele(var));
            } else { // insertion
                var.insert(off+1, alt->data()+1);
                _newGt[e-beg].push_back(addAllele(var));
            }
        }
    }

    _mergedAlt.resize(_alleleMap.size());
    for (auto i = _alleleMap.begin(); i != _alleleMap.end(); ++i)
        _mergedAlt[i->second] = i->first;
}

string AlleleMerger::buildRef(Entry const* beg, Entry const* end) {
    // beg -> end should be sorted by start position
    string ref = beg->ref();
    uint64_t pos = beg->pos() + ref.size();

    for (Entry const* e = beg+1; e != end; ++e) {
        if (pos < e->pos())
            return "";

        if (e->pos() + e->ref().size() < pos)
            continue;

        ptrdiff_t off = pos - e->pos();
        ref += e->ref().data() + off;
        pos += e->ref().size() - off;
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
