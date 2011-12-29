#include "GenotypeCall.hpp" 

#include "common/Tokenizer.hpp"

BEGIN_NAMESPACE(Vcf)

GenotypeCall::GenotypeCall()
    : _phased(false)
{
}

GenotypeCall::GenotypeCall(const std::string& call)
    : _phased(false)
    , _string(call)
{
    Tokenizer<std::string> tok(call, "|/");
    uint32_t idx(0);
    // TODO this doesn't seem to handle partially missing data yet
    // note: a delimiter of | denotes phased data
    // we only pay attention to the first such delimiter
    if (tok.extract(idx)) {
        _phased = tok.lastDelim() == '|';
        _indices.push_back(idx);
        _indexSet.insert(idx);
        // now read the rest
        while (tok.extract(idx)) {
            _indices.push_back(idx);
            _indexSet.insert(idx);
        }
            
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
bool GenotypeCall::operator==(const GenotypeCall& rhs) const {
    if(_phased == rhs._phased) {
        if(_phased) {
            return (_string == rhs._string);
        }
        else {
            //order doesn't matter so need to sort
            return (_indexSet == rhs._indexSet);
        }
    }
    else {
        return false;
    }
}
bool GenotypeCall::operator!=(const GenotypeCall& rhs) const {
    return !(*this == rhs);
}
bool GenotypeCall::operator<(const GenotypeCall& rhs) const {
    return _string < rhs._string; //lame but fingers crossed
}

END_NAMESPACE(Vcf)
