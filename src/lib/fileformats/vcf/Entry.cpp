#include "Entry.hpp"

#include <cassert>
#include <functional>
#include <stdexcept>

using namespace placeholders;
using namespace std;

VCF_NAMESPACE_BEGIN

InfoField::InfoField() {}

InfoField::InfoField(const string& raw) {
    Tokenizer<char> t(raw, '=');
    t.extract(_id);
    t.extract(_value);
}

InfoField::InfoField(const string& id, const string& val)
    : _id(id)
    , _value(val)
{
}

const string& InfoField::id() const {
    return _id;
}

const string& InfoField::value() const {
    return _value;
}

Entry::Entry() {}
Entry::Entry(const string& s) {
    parse(s);
}

Entry::Entry(
    // non variable length fields
    const string& chrom,
    uint64_t pos,
    const string& ref,
    double qual
    )
    : _chrom(chrom)
    , _pos(pos)
    , _ref(ref)
    , _qual(qual)
{
}

void Entry::addIdentifier(const string& id) {
    _identifiers.push_back(id);
}

void Entry::addAlt(const string& alt) {
    _alt.push_back(alt);
}

void Entry::addFailedFilter(const string& filter) {
    _failedFilters.push_back(filter);
}

void Entry::addInfoField(const string& key, const string& value) {
    _info.push_back(InfoField(key, value));
}

void Entry::addFormatDescription(const string& desc) {
    _formatDescription.push_back(desc);
}

void Entry::addPerSampleData(const string& key, const string& value) {
    throw runtime_error("adding per sample data is not yet implemented");
}

void Entry::parse(const string& s) {
    Tokenizer<char> tok(s, '\t');    
    tok.extract(_chrom);
    tok.extract(_pos);

    string tmp;

    // ids
    tok.extract(tmp);
    extractList(_identifiers, tmp);

    // ref alleles
    tok.extract(_ref);

    // alt alleles
    tok.extract(tmp);
    extractList(_alt, tmp, ',');

    // phred quality
    tok.extract(_qual);

    // failed filters
    tok.extract(tmp);
    extractList(_failedFilters, tmp);
    if (_failedFilters.size() == 1 && _failedFilters[0] == "PASS")
        _failedFilters.clear();

    // info entries
    tok.extract(tmp);
    extractList(_info, tmp);

    // format description
    tok.extract(tmp);
    extractList(_formatDescription, tmp);

    // per sample formatted data
    while (tok.extract(tmp)) {
        // TODO: less copying
        vector<string> data;
        extractList(data, tmp);
        _perSampleData.push_back(data);
    }
}

string Entry::toString() const {
    stringstream ss;
    ss << *this;
    return ss.str();
}

VCF_NAMESPACE_END

ostream& operator<<(ostream& s, const Vcf::InfoField& i) {
    s << i.id();
    if (!i.value().empty())
        s << '=' << i.value();
    return s;
}

ostream& operator<<(ostream& s, const Vcf::Entry& e) {
    s << e.chrom() << '\t' << e.pos() << '\t';
    e.printList(s, e.identifiers());
    s << '\t' << e.ref() << '\t';
    e.printList(s, e.alt(), ',');
    s << '\t' << e.qual() << '\t';
    if (e.failedFilters().empty())
        s << "PASS";
    else
        e.printList(s, e.failedFilters());
    s << '\t';

    e.printList(s, e.info());
    s << '\t';
    e.printList(s, e.formatDescription());
    const vector< vector<string> >& psd = e.perSampleData();
    for (auto i = psd.begin(); i != psd.end(); ++i) {
        s << '\t';
        e.printList(s, *i);
    }
    return s;
}

