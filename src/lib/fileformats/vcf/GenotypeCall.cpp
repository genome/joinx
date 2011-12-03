#include "GenotypeCall.hpp" 

#include "common/Tokenizer.hpp"

BEGIN_NAMESPACE(Vcf)

GenotypeCall::GenotypeCall()
    : _phased(false)
{
}

GenotypeCall::GenotypeCall(const string& call)
    : _phased(false)
{
    Tokenizer<string> tok(call, "|/");
    uint32_t idx(0);
    // note: a delimiter of | denotes phased data
    // we only pay attention to the first such delimiter
    if (tok.extract(idx)) {
        _phased = tok.lastDelim() == '|';
        _indices.push_back(idx);
        // now read the rest
        while (tok.extract(idx))
            _indices.push_back(idx);
    }
}

bool GenotypeCall::empty() const {
    return _indices.empty();
}

GenotypeCall::size_type GenotypeCall::size() const {
    return _indices.size();
}

GenotypeCall::const_iterator GenotypeCall::begin() const {
    return _indices.begin();
}

GenotypeCall::const_iterator GenotypeCall::end() const {
    return _indices.end();
}

bool GenotypeCall::phased() const {
    return _phased;
}

const uint32_t& GenotypeCall::operator[](size_type idx) const {
    return _indices[idx];    
}

const vector<uint32_t>& GenotypeCall::indices() const {
    return _indices;
}

END_NAMESPACE(Vcf)
