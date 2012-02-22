#include "Entry.hpp"
#include "EntryMerger.hpp"
#include "CustomValue.hpp"
#include "GenotypeFormatter.hpp"
#include "Header.hpp"
#include "MergeStrategy.hpp"
#include "common/Sequence.hpp"

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <algorithm>
#include <cassert>
#include <cstring>
#include <functional>
#include <iterator>
#include <limits>
#include <stdexcept>
#include <utility>

using boost::format;
using boost::lexical_cast;
using namespace placeholders;
using namespace std;

BEGIN_NAMESPACE(Vcf)
namespace {
    const static char* entryFieldNames[] = {
        "chrom",
        "pos",
        "id",
        "ref",
        "alt",
        "qual",
        "filter",
        "info",
        "format",
        "sample_data"
    };
}

const double Entry::MISSING_QUALITY = numeric_limits<double>::min();

bool Entry::posLess(Entry const& a, Entry const& b) {
    return a.pos() < b.pos();
}

bool Entry::refStopLess(Entry const& a, Entry const& b) {
    return a.refStop() < b.refStop();
}

bool Entry::chromEq(const std::string& chrom, Entry const& b) {
    return chrom == b.chrom();
}

void Entry::parseLine(const Header* hdr, std::string& s, Entry& e) {
    e.parse(hdr, s);
}

void Entry::parseLineAndReheader(const Header* hdr, const Header* newH, std::string& s, Entry& e) {
    e.parseAndReheader(hdr, newH, s);
}

const char* Entry::fieldToString(FieldName field) {
    if (field >= UNDEFINED)
        return 0;

    return entryFieldNames[field];
}

Entry::FieldName Entry::fieldFromString(const char* name) {
    for (int i = 0; i < UNDEFINED; ++i) {
        if (name == entryFieldNames[i])
            return FieldName(i);
    }
    return UNDEFINED;
}

Entry::Entry()
    : _header(0)
    , _pos(0)
    , _qual(MISSING_QUALITY)
    , _parsedSamples(false)
    , _start(0)
    , _stop(0)
{
}

Entry::Entry(Entry const& e) throw ()
    : _header(e._header)
    , _chrom(e._chrom)
    , _pos(e._pos)
    , _identifiers(e._identifiers)
    , _ref(e._ref)
    , _alt(e._alt)
    , _qual(e._qual)
    , _failedFilters(e._failedFilters)
    , _info(e._info)
    , _sampleString(e._sampleString)
    , _parsedSamples(e._parsedSamples)
    , _sampleData(e._sampleData)
    , _start(e._start)
    , _stop(e._stop)
{
}

Entry::Entry(Entry&& e) throw ()
    : _header(e._header)
    , _chrom(std::move(e._chrom))
    , _pos(e._pos)
    , _identifiers(std::move(e._identifiers))
    , _ref(std::move(e._ref))
    , _alt(std::move(e._alt))
    , _qual(e._qual)
    , _failedFilters(std::move(e._failedFilters))
    , _info(std::move(e._info))
    , _sampleString(std::move(e._sampleString))
    , _parsedSamples(e._parsedSamples)
    , _sampleData(std::move(e._sampleData))
    , _start(e._start)
    , _stop(e._stop)
{
}

Entry::Entry(const Header* h)
    : _header(h)
    , _pos(0)
    , _qual(MISSING_QUALITY)
    , _parsedSamples(false)
    , _start(0)
    , _stop(0)
{
}

Entry::Entry(const Header* h, const string& s)
    : _header(h)
    , _qual(MISSING_QUALITY)
    , _parsedSamples(false)
    , _start(0)
    , _stop(0)
{
    parse(h, s);
}

Entry::Entry(EntryMerger&& merger)
    : _header(merger.mergedHeader())
    , _chrom(merger.chrom())
    , _pos(merger.pos())
    , _identifiers(std::move(merger.identifiers()))
    , _ref(merger.ref())
    , _qual(merger.qual())
    , _failedFilters(std::move(merger.failedFilters()))
    , _parsedSamples(true)
    , _start(0)
    , _stop(0)
{
    if (merger.entryCount() < 2) {
        throw runtime_error(str(format(
            "Logic error: attempted to merge a single entry: %1%"
            ) %merger.entries()->toString()));
    }
    if (!merger.merged()) {
        stringstream ss;
        for (size_t i = 0; i < merger.entryCount(); ++i) {
            ss << merger.entries()[i] << "\n";
        }
        throw runtime_error(str(format("Failed to merge entries:\n %1%") %ss.str()));
    }

    merger.setInfo(_info);
    merger.setAltAndGenotypeData(_alt, _sampleData);
    setPositions();
}

void Entry::reheader(const Header* newHeader) {
    sampleData().reheader(newHeader);
    _header = newHeader;
}

const Header& Entry::header() const {
    if (!_header)
        throw runtime_error("Attempted to use Vcf Entry with no header!");
    return *_header;
}

void Entry::parseAndReheader(const Header* h, const Header* newHeader, const string& s) {
    parse(h, s);
    reheader(newHeader);
}

void Entry::parse(const Header* h, const string& s) {
    _parsedSamples = false;
    _header = h;

    // clear containers
    _info.clear();
    _sampleData.clear();
    _identifiers.clear();
    _alt.clear();
    _failedFilters.clear();

    Tokenizer<char> tok(s, '\t');
    if (!tok.extract(_chrom))
        throw runtime_error("Failed to extract chromosome from vcf entry: " + s);
    if (!tok.extract(_pos))
        throw runtime_error("Failed to extract position from vcf entry: " + s);

    char const* beg(0);
    char const* end(0);

    // ids
    if (!tok.extract(&beg, &end))
        throw runtime_error("Failed to extract id from vcf entry: " + s);

    if (end-beg != 1 || *beg != '.')
        Tokenizer<char>::split(beg, end, ';', inserter(_identifiers, _identifiers.begin()));

    // ref alleles
    if (!tok.extract(_ref))
        throw runtime_error("Failed to extract ref alleles from vcf entry: " + s);

    // alt alleles
    if (!tok.extract(&beg, &end))
        throw runtime_error("Failed to extract alt alleles from vcf entry: " + s);

    if (end-beg != 1 || *beg != '.')
        Tokenizer<char>::split(beg, end, ',', back_inserter(_alt));

    // phred quality
    string qualstr;
    if (!tok.extract(qualstr))
        throw runtime_error("Failed to extract quality from vcf entry: " + s);
    if (qualstr == ".")
        _qual = MISSING_QUALITY;
    else
        _qual = lexical_cast<double>(qualstr);

    // failed filters
    if (!tok.extract(&beg, &end))
        throw runtime_error("Failed to extract filters from vcf entry: " + s);

    if (end-beg != 1 || *beg != '.')
        Tokenizer<char>::split(beg, end, ';', inserter(_failedFilters,_failedFilters.end()));


    // If pass is present as well as other failed filters, remove pass
    if (_failedFilters.size() > 1) {
        _failedFilters.erase("PASS");
    }

    // info entries
    if (!tok.extract(&beg, &end))
        throw runtime_error("Failed to extract info from vcf entry: " + s);

    vector<string> infoStrings;
    if (end-beg != 1 || *beg != '.')
        Tokenizer<char>::split(beg, end, ';', back_inserter(infoStrings));

    // TODO: refactor into function addInfoField(s)
    for (auto i = infoStrings.begin(); i != infoStrings.end(); ++i) {
        if (i->empty())
            continue;

        Tokenizer<char> kv(*i, '=');
        string key;
        string value;
        kv.extract(key);
        kv.remaining(value);
        const CustomType* type = header().infoType(key);
        if (type == NULL)
            throw runtime_error(str(format("Failed to lookup type for info field '%1%'") %key));
        CustomValue cv(type, value);
        cv.setNumAlts(_alt.size());
        auto inserted = _info.insert(make_pair(key, CustomValue(type, value)));
        if (!inserted.second)
            throw runtime_error(str(format("Duplicate value for info field '%1%'") %key));
    }

    tok.remaining(_sampleString);
    _parsedSamples = false;
    setPositions();
}

void Entry::addIdentifier(const std::string& id) {
    _identifiers.insert(id);
}

void Entry::addFilter(const std::string& filterName) {
    //filters cannot contain whitespace or semicolons
    std::string::const_iterator it = find_if(filterName.begin(),filterName.end(),isInvalidFilterId);
    if( it != filterName.end()) {
        //then it is invalid
        throw runtime_error(str(format("Invalid filter name %1%. Contains whitespace or a semicolon.") %filterName ));
    }

    if (_failedFilters.find("PASS") != _failedFilters.end()) {
        _failedFilters.erase("PASS");
    }

    _failedFilters.insert(filterName);
}

string Entry::toString() const {
    stringstream ss;
    ss << *this;
    return ss.str();
}

int Entry::cmp(const Entry& rhs) const {
    int rv = strverscmp(_chrom.c_str(), rhs._chrom.c_str());
    if (rv != 0)
        return rv;

    if (_pos < rhs._pos)
        return -1;
    if (rhs._pos < _pos)
        return 1;

    return 0;
}

void Entry::swap(Entry& other) {
    _chrom.swap(other._chrom);
    std::swap(_pos, other._pos);
    _identifiers.swap(other._identifiers);
    _ref.swap(other._ref);
    _alt.swap(other._alt);
    std::swap(_qual, other._qual);
    _failedFilters.swap(other._failedFilters);
    _info.swap(other._info);
    _sampleData.swap(other._sampleData);
    std::swap(_header, other._header);
    std::swap(_parsedSamples, other._parsedSamples);
    _sampleString.swap(other._sampleString);
    setPositions();
}

int32_t Entry::altIdx(const string& alt) const {
    auto i = find(_alt.begin(), _alt.end(), alt);
    if (i == _alt.end())
        return -1;
    return distance(_alt.begin(), i);
}

const CustomValue* Entry::info(const string& key) const {
    auto i = _info.find(key);
    if (i == _info.end())
        return 0;
    return &i->second;
}

void Entry::setInfo(std::string const& key, CustomValue const& value) {
    auto i = _info.find(key);
    if (i == _info.end()) {
        _info.insert(make_pair(key, value));
    } else {
        i->second = value;
    }
}


SampleData& Entry::sampleData() {
    if (!_parsedSamples) {
        _sampleData = SampleData(_header, _sampleString);
        _parsedSamples = true;
    }

    return _sampleData;
}

const SampleData& Entry::sampleData() const {
    if (!_parsedSamples) {
        _sampleData = SampleData(_header, _sampleString);
        _parsedSamples = true;
    }

    return _sampleData;
}

void Entry::setPositions() {
    if (_alt.empty()) {
        _start = _pos - 1;
        _stop = _pos;
        return;
    }

    _start = numeric_limits<int64_t>::max();
    _stop = 0;
    for (uint32_t idx = 0; idx < _alt.size(); ++idx) {
        if (_ref == _alt[idx]) {
            throw runtime_error(str(format(
                "Nonsense variant: identical to reference in %1%"
                ) % toString() ));
        }

        string::size_type prefix = Sequence::commonPrefix(_ref, _alt[idx]);
        int64_t start = _pos - 1 + prefix;
        int64_t stop;
        if (_alt[idx].size() == _ref.size()) {
            stop = start + _alt[idx].size() - prefix;
        } else {
            // VCF prepends 1 base to indels
            if (_alt[idx].size() < _ref.size()) { // deletion
                stop = start + _ref.size();
            } else if (_alt[idx].size() > _ref.size()) { // insertion
                ++start;
                stop = start;
            } else // let's see if this ever happens!
                throw runtime_error(str(format("Unknown variant type, allele %1%: %2%") %idx %toString()));
        }
        _start = min(_start, start);
        _stop = max(_stop, stop);
    }
}

int64_t Entry::start() const {
    return _start;
}

int64_t Entry::stop() const {
    return _stop;
}

END_NAMESPACE(Vcf)

ostream& operator<<(ostream& s, const Vcf::Entry& e) {
    s << e.chrom() << '\t' << e.pos() << '\t';
    e.printList(s, e.identifiers());
    s << '\t' << e.ref() << '\t';
    e.printList(s, e.alt(), ',');
    if (e.qual() <= Vcf::Entry::MISSING_QUALITY)
        s << "\t.\t";
    else
        s << '\t' << e.qual() << '\t';

    e.printList(s, e.failedFilters());
    s << '\t';

    const Vcf::Entry::CustomValueMap& info = e.info();
    if (info.empty()) {
        s << '.';
    } else {
        for (auto i = info.begin(); i != info.end(); ++i) {
            if (i != info.begin())
                s << ';';
            s << i->second.type().id();
            string value = i->second.toString();
            if (!i->second.empty()) {
                s << "=" << value;
            }
        }
    }
    s << '\t' << e.sampleData();

    return s;
}
