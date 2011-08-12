#include "VariantAdaptor.hpp"

#include <boost/format.hpp>
#include <algorithm>
#include <cassert>
#include <vector>
#include <stdexcept>

using boost::format;
using namespace std;

VCF_NAMESPACE_BEGIN

VariantAdaptor::VariantAdaptor(const Entry& entry)
    : _entry(entry)
{
    const vector<string>& alts = _entry.alt();
    _start = _stop = _entry.pos();
    for (uint32_t idx = 0; idx < _entry.alt().size(); ++idx) {
        string::size_type prefix = commonPrefix(_entry.ref(), alts[idx]);
        int64_t start = _entry.pos() - 1 + prefix;
        int64_t stop;
        if (alts[idx].size() == _entry.ref().size()) {
            stop = start + alts[idx].size() - prefix;
        } else {
            // VCF prepends 1 base to indels
            if (alts[idx].size() < _entry.ref().size()) { // deletion
                stop = start + _entry.ref().size();
            } else if (alts[idx].size() > _entry.ref().size()) { // insertion
                ++start;
                stop = start;
            } else // let's see if this ever happens!
                throw runtime_error(str(format("Unknown variant type, allele %1%: %2%") %idx %_entry.toString()));
        }
        _start = min(_start, start);
        _stop = max(_stop, stop);
    }
}

string::size_type VariantAdaptor::commonPrefix(const string& a, const string& b) {
    string::size_type p = 0; 
    while (p < a.size() && p < b.size() && a[p] == b[p])
        ++p;
    return p;
}

VCF_NAMESPACE_END
