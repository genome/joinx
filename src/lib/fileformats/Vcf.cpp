#include "Vcf.hpp"

#include <boost/format.hpp>
#include <algorithm>
#include <functional>
#include <sstream>
#include <stdexcept>

using namespace std;
using namespace std::placeholders;
using boost::format;

VCF_NAMESPACE_BEGIN

namespace {
    struct StrType {
        string txt;
        DataType type;

        bool operator<(const StrType& rhs) const { return txt < rhs.txt; }
        bool operator<(const string& rhs) const { return txt < rhs; }
        bool operator==(const StrType& rhs) const { return txt == rhs.txt; }
        bool operator==(const string& rhs) const { return txt == rhs; }
    } __str_types[] = {
        {"Character", CHAR},
        {"Flag", FLAG},
        {"Float", FLOAT},
        {"Integer", INT},
        {"String", STRING},
    };

    const uint32_t __n_types = sizeof(__str_types)/sizeof(__str_types[0]);
}


DataType strToType(const string& s) {
    StrType* p = lower_bound(__str_types, __str_types+__n_types, s);
    if (p != __str_types+__n_types && p->txt == s)
        return p->type;
    throw runtime_error(str(format("Unknown data type '%1%'") %s));
}

string parseString(const string& s) {
    if (s.size() < 2)
        throw runtime_error(str(format("Invalid vcf string: %1%") %s));

    if (s[0] != '"' || s[s.size()-1] != '"')
        throw runtime_error(str(format("Unquoted vcf string: %1%") %s));

    string tmp = s.substr(1, s.size()-2);

    string::size_type lastPos = 0;
    stringstream ss;
    string::size_type pos;
    do {
        pos = tmp.find('\\', lastPos);
        if (pos == tmp.size() - 1)
            throw runtime_error(str(format("Trailing backslash in vcf string: %1%") %s));
        ss << tmp.substr(lastPos, pos-lastPos);
        if (pos != string::npos)
            ss << tmp[pos+1];
        lastPos = pos+2;
        string curr = ss.str();
        string rest = tmp.substr(lastPos);
    } while (pos < tmp.size());
    return ss.str();
}


Header::PropLine::PropLine() {
}

Header::PropLine::PropLine(const std::string& s, const std::vector<std::string> validProps) {
    Tokenizer<char> t(s, ',');
    string tmp;
    while (t.extract(tmp)) {
        Tokenizer<char> eqt(tmp, '=');
        string key;
        string value;
        if (!eqt.extract(key) || !eqt.extract(value) || !eqt.eof())
            throw runtime_error(str(format("Failed to parse VCF header line: %1%") %s));
        if (find(validProps.begin(), validProps.end(), key) == validProps.end())
            throw runtime_error(str(format("Unexpected key in VCF header line (%1%): %2%") %key %s));
        _keyOrder.push_back(key);
        typedef map<string,string>::iterator IterType;
        pair<IterType, bool> inserted = _map.insert(make_pair(key, value));
        if (!inserted.second)
            throw runtime_error(str(format("Duplicate key in VCF header line (%1%): %2%") %key %s));
    } 
}

const std::vector<std::string>& Header::PropLine::keys() const {
    return _keyOrder;
}

const std::string& Header::PropLine::operator[](const std::string& key) {
    return _map[key];
}

Header::Header() {}

void Header::add(const string& line) {
    Tokenizer<char> t(line, '=');
    pair<string, string> p;
    if (!t.extract(p.first))
        throw runtime_error(str(format("Failed to parse VCF header line: %1%") %line));
    t.remaining(p.second);
    _lines.push_back(p);
}

const vector<Header::RawLine>& Header::lines() const {
    return _lines;
}

InfoField::InfoField() {}

InfoField::InfoField(const std::string& raw) {
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

int64_t Entry::start() const {
    return _pos;
}

int64_t Entry::stop() const {
    auto longestAlt = max_element(_alt.begin(), _alt.end(), bind(&string::size, _1));
    if (longestAlt == _alt.end())
        return _pos;
    return _pos + longestAlt->size();
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
    extractList(_alt, tmp);

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

vector<Variant> Entry::variants() const {
    vector<Variant> rv;
    // VCF includes the base before the variant, we strip that off 
    string refp = _ref.substr(1);
    for (auto i = _alt.begin(); i != _alt.end(); ++i) {
        string altp = i->substr(1);
        int64_t start = _pos;
        int64_t end = _pos + altp.size();
        rv.push_back(Variant(_chrom, start, end, _qual, 0, refp, altp));
    }
    return rv;
}

string Entry::toString() const {
    stringstream ss;
    ss << *this;
    return ss.str();
}

VCF_NAMESPACE_END

std::ostream& operator<<(std::ostream& s, const Vcf::Header& h) {
    const vector<Vcf::Header::RawLine>& lines = h.lines();
    for (auto i = lines.begin(); i != lines.end(); ++i)
        s << i->first << "=" << i->second << "\n";
    return s;
}

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
    e.printList(s, e.alt());
    s << '\t' << e.qual() << '\t';
    if (e.failedFilters().empty())
        s << "PASS";
    else
        e.printList(s, e.failedFilters());
    s << '\t';

    e.printList(s, e.info());
    s << '\t';
    e.printList(s, e.formatDescription());
    s << '\t';
    const vector< vector<string> >& psd = e.perSampleData();
    for (auto i = psd.begin(); i != psd.end(); ++i) {
        if (i != psd.begin())
            s << '\t';
        e.printList(s, *i);
    }
    return s;
}
