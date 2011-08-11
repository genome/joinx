#include "VariantAdaptor.hpp"

#include <boost/format.hpp>
#include <cassert>
#include <vector>
#include <stdexcept>

using boost::format;
using namespace std;

VCF_NAMESPACE_BEGIN

VariantAdaptor::VariantAdaptor(const Entry& entry)
    : _entry(entry)
    , _idx(0)
    , _ref(entry.ref())
{
    setIndex(_idx);
}

string::size_type VariantAdaptor::commonPrefix(const string& a, const string& b) {
    string::size_type p = 0; 
    while (p < a.size() && p < b.size() && a[p] == b[p])
        ++p;
    return p;
}


bool VariantAdaptor::advance() {
    if (++_idx < _entry.alt().size()) {
        setIndex(_idx);
        return true;
    }
    return false;
}

void VariantAdaptor::setIndex(uint32_t idx) {
    // test if snp or dnp, etc
    const vector<string>& alt = _entry.alt();
    if (idx >= alt.size()) {
        _start = _stop = 0;
        _alt = _ref = "";
        return;
    }

    string::size_type prefix = commonPrefix(_entry.ref(), alt[_idx]);
    _start = _entry.pos() - 1 + prefix;
    if (alt[_idx].size() == _entry.ref().size()) {
        _stop = _start + alt[_idx].size() - prefix;
        _ref = _entry.ref().substr(prefix, alt[_idx].size() - prefix);
        _alt = alt[_idx].substr(prefix);
        assert(_ref.size() == _alt.size());
    } else {
        _ref = _entry.ref().substr(prefix);
        _alt = alt[_idx].substr(prefix);
        // VCF prepends 1 base to indels
        if (_alt.size() < _ref.size()) { // deletion
            _stop = _start + _ref.size();
        } else if (_alt.size() > _ref.size()) { // insertion
            ++_start;
            _stop = _start;
        } else // let's see if this ever happens!
            throw runtime_error(str(format("Unknown variant type, allele %1%: %2%") %_idx %_entry.toString()));
    }
}

VCF_NAMESPACE_END
