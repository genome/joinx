#include "Entry.hpp"
#include "EntryMerger.hpp"
#include "CustomValue.hpp"
#include "GenotypeFormatter.hpp"
#include "Header.hpp"
#include "MergeStrategy.hpp"

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <algorithm>
#include <cassert>
#include <cstring>
#include <functional>
#include <iterator>
#include <limits>
#include <stdexcept>

using boost::format;
using boost::lexical_cast;
using namespace placeholders;
using namespace std;

BEGIN_NAMESPACE(Vcf)
namespace {
    string::size_type commonPrefix(const string& a, const string& b) {
        string::size_type p = 0;
        while (p < a.size() && p < b.size() && a[p] == b[p])
            ++p;
        return p;
    }

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

    bool customTypeIdMatches(string const& id, CustomType const* type) {
        return type && type->id() == id;
    }
}

void Entry::parseLine(const Header* hdr, std::string& s, Entry& e) {
    e._parsedSamples = false;
    e.parse(hdr, s);
}

void Entry::parseLineAndReheader(const Header* hdr, const Header* newH, std::string& s, Entry& e) {
    e._parsedSamples = false;
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



const double Entry::MISSING_QUALITY = numeric_limits<double>::min();

Entry::Entry()
    : _header(0)
    , _pos(0)
    , _qual(MISSING_QUALITY)
    , _parsedSamples(false)
    , _start(0)
    , _stop(0)
{
}

Entry::Entry(Entry&& e) throw () {
    swap(e);
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
    , _ref(merger.ref())
    , _qual(merger.qual())
    , _parsedSamples(true)
    , _start(0)
    , _stop(0)
{
    std::swap(_identifiers, merger.identifiers());
    std::swap(_failedFilters, merger.failedFilters());
    merger.setInfo(_info);
    merger.setAltAndGenotypeData(_alt, _formatDescription, _sampleData);
    setPositions();
}

void Entry::reheader(const Header* newHeader) {
    SampleData newGTData;
    auto const& sd = sampleData();
    for (auto i = sd.begin(); i != sd.end(); ++i) {
        const string& sampleName = header().sampleNames()[i->first];
        uint32_t newIdx = newHeader->sampleIndex(sampleName);
        newGTData[newIdx] = i->second;
    }
    _sampleData.swap(newGTData);
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
    _header = h;

    // clear containers
    _info.clear();
    _sampleData.clear();
    _identifiers.clear();
    _alt.clear();
    _failedFilters.clear();
    _formatDescription.clear();

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
        auto inserted = _info.insert(make_pair(key, CustomValue(type, value)));
        if (!inserted.second)
            throw runtime_error(str(format("Duplicate value for info field '%1%'") %key));
    }

    // TODO: refactor into function
    // format description
    vector<string> formatDesc;
    if (tok.extract(&beg, &end) && (end - beg != 1 || *beg != '.')) {
        Tokenizer<char>::split(beg, end, ':', back_inserter(formatDesc));

        _formatDescription.reserve(formatDesc.size());
        for (auto i = formatDesc.begin(); i != formatDesc.end(); ++i) {
            if (i->empty())
                continue;

            auto type = header().formatType(*i);
            if (!type)
                throw runtime_error(str(format("Unknown id in FORMAT field: %1%") %*i));
            _formatDescription.push_back(type);
        }
    }

    tok.remaining(_sampleString);
    setPositions();
}

void Entry::parseSamples() const {
    if (_parsedSamples)
        return;

    // per sample formatted data
    uint32_t sampleIdx(0);
    Tokenizer<char> tok(_sampleString, '\t');
    char const* beg(0);
    char const* end(0);
    while (tok.extract(&beg, &end)) {
        vector<string> data;
        if (end-beg != 1 || *beg != '.') {
            Tokenizer<char>::split(beg, end, ':', back_inserter(data));

            if (data.size() > _formatDescription.size())
                throw runtime_error("More per-sample values than described in format section");

            vector<CustomValue>& values = (_sampleData[sampleIdx] = vector<CustomValue>());
            values.resize(data.size());
            for (uint32_t i = 0; i < data.size(); ++i) {
                values[i] = CustomValue(_formatDescription[i], data[i]);
            }
        }
        ++sampleIdx;
    }
    _parsedSamples = true;
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
    _formatDescription.swap(other._formatDescription);
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

const Entry::SampleData& Entry::sampleData() const {
    parseSamples();
    return _sampleData;
}

Entry::SampleData& Entry::sampleData() { // protected
    parseSamples();
    return _sampleData;
}

const vector<CustomValue>* Entry::sampleData(uint32_t idx) const {
    auto const& sd = sampleData();
    auto iter = sd.find(idx);
    if (iter == sd.end())
        return 0;
    return &iter->second;
}

const CustomValue* Entry::sampleData(uint32_t sampleIdx, const string& key) const {
    auto const& sd = sampleData();
    // no data for that sample
    auto iter = sd.find(sampleIdx);
    if (iter == sd.end())
        return 0;

    // no info for that format key
    auto i = find_if(_formatDescription.begin(), _formatDescription.end(), bind(&customTypeIdMatches, key, _1));
    if (i == _formatDescription.end())
        return 0;

    uint32_t offset = distance(_formatDescription.begin(), i);
    if (offset >= iter->second.size())
        return 0;

    return &iter->second[offset];
}

bool Entry::hasGenotypeData() const {
    return !_formatDescription.empty() && _formatDescription.front()->id() == "GT";
}

GenotypeCall Entry::genotypeForSample(uint32_t sampleIdx) const {
    const string* gtString(0);
    const CustomValue* v = sampleData(sampleIdx, "GT");
    if (!v || v->empty() || (gtString = v->get<string>(0)) == 0 || gtString->empty())
        return GenotypeCall();
    return GenotypeCall(*gtString);
}

void Entry::removeLowDepthGenotypes(uint32_t lowDepth) {
    auto i = find_if(_formatDescription.begin(), _formatDescription.end(), bind(&customTypeIdMatches, "DP", _1));
    if (i == _formatDescription.end())
        return;

    auto& sd = sampleData();
    uint32_t offset = distance(_formatDescription.begin(), i);
    for (auto i = sd.begin(); i != sd.end(); ++i) {
        const int64_t *v;
        if (i->second[offset].empty() || (v = i->second[offset].get<int64_t>(0)) == NULL || *v < lowDepth)
            i->second.clear();
    }
}

uint32_t Entry::samplesWithData() const {
    uint32_t rv(0);
    auto const& sd = sampleData();
    for (auto i = sd.begin(); i != sd.end(); ++i)
        if (!i->second.empty())
            ++rv;
    return rv;
}

int32_t Entry::samplesFailedFilter() const {
    auto i = find_if(_formatDescription.begin(), _formatDescription.end(), bind(&customTypeIdMatches, "FT", _1));
    if (i == _formatDescription.end())
        return -1;

    uint32_t offset = distance(_formatDescription.begin(), i);
    uint32_t numFailedFilter = 0;
    auto const& sd = sampleData();
    for (auto i = sd.begin(); i != sd.end(); ++i) {
        auto const& values = i->second;
        if (values.size() > offset) {
            //then we have some data
            const std::string *filter;
            //if it has a value (assume . is processed correctly) and we're able to get a value and it is not pass then failed
            if (!values[offset].empty() && (filter = values[offset].get<std::string>(0)) != NULL && *filter != std::string("PASS")) {
               numFailedFilter++;
            }
        }
    }
    return numFailedFilter;
}

int32_t Entry::samplesEvaluatedByFilter() const {
    auto i = find_if(_formatDescription.begin(), _formatDescription.end(), bind(&customTypeIdMatches, "FT", _1));
    if (i == _formatDescription.end())
        return -1;

    uint32_t offset = distance(_formatDescription.begin(), i);
    uint32_t numEvaluatedByFilter = 0;
    auto const& sd = sampleData();
    for (auto i = sd.begin(); i != sd.end(); ++i) {
        auto const& values = i->second;
        if (values.size() > offset) {
            //then we have some data
            //if it has a value (assume . is processed correctly) and we're able to get a value and it is not pass then failed
            if (!values[offset].empty() && values[offset].get<std::string>(0) != NULL) {
               numEvaluatedByFilter++;
            }
        }
    }
    return numEvaluatedByFilter;
}

void Entry::setPositions() {
    _start = _stop = _pos;
    for (uint32_t idx = 0; idx < _alt.size(); ++idx) {
        string::size_type prefix = commonPrefix(_ref, _alt[idx]);
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
    s << '\t';

    auto const& fmt = e.formatDescription();
    if (!fmt.empty()) {
        for (auto i = fmt.begin(); i != fmt.end(); ++i) {
            if (i != fmt.begin())
                s << ':';
            s << (*i)->id();
        }
    } else {
        s << '.';
    }

    Vcf::Entry::SampleData const& psd = e.sampleData();
    uint32_t sampleCounter(0);
    uint32_t nSamples = e.header().sampleCount();
    for (auto i = psd.begin(); i != psd.end(); ++i) {
        s << '\t';
        while (sampleCounter < i->first) {
            s << ".\t";
            ++sampleCounter;
        }

        auto const& values = i->second;
        if (!values.empty()) {
            for (auto j = values.begin(); j != values.end(); ++j) {
                if (j != values.begin())
                    s << ':';
                if (j->empty())
                    s << '.';
                else
                    j->toStream(s);
            }
        } else
            s << ".";
        ++sampleCounter;
    }
    while (sampleCounter++ < nSamples)
        s << "\t.";

    return s;
}
