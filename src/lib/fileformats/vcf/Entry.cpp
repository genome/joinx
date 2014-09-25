#include "Entry.hpp"
#include "EntryMerger.hpp"
#include "CustomValue.hpp"
#include "Header.hpp"
#include "MergeStrategy.hpp"
#include "common/String.hpp"
#include "io/StreamJoin.hpp"

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
using namespace std;

namespace {
    std::string const MISSING_STRING = ".";
}

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
    , _startWithoutPadding(0)
    , _stopWithoutPadding(0)
    , _qual(MISSING_QUALITY)
    , _parsedSamples(false)
{
}

Entry::Entry(Entry const& e)
    : _header(e._header)
    , _chrom(e._chrom)
    , _pos(e._pos)
    , _startWithoutPadding(e._startWithoutPadding)
    , _stopWithoutPadding(e._stopWithoutPadding)
    , _identifiers(e._identifiers)
    , _ref(e._ref)
    , _alt(e._alt)
    , _qual(e._qual)
    , _failedFilters(e._failedFilters)
    , _info(e._info)
    , _sampleString(e._sampleString)
    , _parsedSamples(e._parsedSamples)
    , _sampleData(e._sampleData)
{
}

Entry::Entry(Entry&& e)
    : _header(e._header)
    , _chrom(std::move(e._chrom))
    , _pos(e._pos)
    , _startWithoutPadding(e._startWithoutPadding)
    , _stopWithoutPadding(e._stopWithoutPadding)
    , _identifiers(std::move(e._identifiers))
    , _ref(std::move(e._ref))
    , _alt(std::move(e._alt))
    , _qual(e._qual)
    , _failedFilters(std::move(e._failedFilters))
    , _info(std::move(e._info))
    , _sampleString(std::move(e._sampleString))
    , _parsedSamples(e._parsedSamples)
    , _sampleData(std::move(e._sampleData))
{
}

Entry& Entry::operator=(Entry const& e) {
    _header = e._header;
    _chrom = e._chrom;
    _pos = e._pos;
    _startWithoutPadding = e._startWithoutPadding;
    _stopWithoutPadding = e._stopWithoutPadding;
    _identifiers = e._identifiers;
    _ref = e._ref;
    _alt = e._alt;
    _qual = e._qual;
    _failedFilters = e._failedFilters;
    _info = e._info;
    _sampleString = e._sampleString;
    _parsedSamples = e._parsedSamples;
    _sampleData = e._sampleData;
    return *this;
}

Entry& Entry::operator=(Entry&& e) {
    _header = std::move(e._header);
    _chrom = std::move(e._chrom);
    _pos = e._pos;
    _startWithoutPadding = e._startWithoutPadding;
    _stopWithoutPadding = e._stopWithoutPadding;
    _identifiers = std::move(e._identifiers);
    _ref = std::move(e._ref);
    _alt = std::move(e._alt);
    _qual = std::move(e._qual);
    _failedFilters = std::move(e._failedFilters);
    _info = std::move(e._info);
    _sampleString = std::move(e._sampleString);
    _parsedSamples = std::move(e._parsedSamples);
    _sampleData = std::move(e._sampleData);
    return *this;
}

Entry::Entry(const Header* h)
    : _header(h)
    , _pos(0)
    , _startWithoutPadding(0)
    , _stopWithoutPadding(0)
    , _qual(MISSING_QUALITY)
    , _parsedSamples(false)
{
}

Entry::Entry(const Header* h, const string& s)
    : _header(h)
    , _startWithoutPadding(0)
    , _stopWithoutPadding(0)
    , _qual(MISSING_QUALITY)
    , _parsedSamples(false)
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
{
    if (!merger.merged()) {
        stringstream ss;
        for (size_t i = 0; i < merger.entryCount(); ++i) {
            ss << merger.entries()[i] << "\n";
        }
        throw runtime_error(str(format("Failed to merge entries:\n %1%") %ss.str()));
    }

    merger.setAltAndGenotypeData(_alt, _sampleData);
    merger.setInfo(_info.get(*_header));

    computeStartStop();
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

    _info.fromString(beg, end);

    tok.remaining(_sampleString);
    _parsedSamples = false;
    computeStartStop();
}

void Entry::addIdentifier(const std::string& id) {
    _identifiers.insert(id);
}

void Entry::addFilter(const std::string& filterName) {
    // filters cannot contain whitespace or semicolons
    auto it = find_if(filterName.begin(), filterName.end(), isInvalidFilterId);
    if (it != filterName.end()) {
        // then it is invalid
        throw runtime_error(str(format(
            "Invalid filter name %1%. Contains whitespace or a semicolon."
            ) % filterName));
    }

    if (_failedFilters.find("PASS") != _failedFilters.end()) {
        _failedFilters.erase("PASS");
    }

    _failedFilters.insert(filterName);
}

void Entry::clearFilters() {
    _failedFilters.clear();
}

string Entry::toString() const {
    stringstream ss;
    ss << *this;
    return ss.str();
}

void Entry::swap(Entry& other) {
    _chrom.swap(other._chrom);
    std::swap(_pos, other._pos);
    std::swap(_startWithoutPadding, other._startWithoutPadding);
    std::swap(_stopWithoutPadding, other._stopWithoutPadding);
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
}

int32_t Entry::altIdx(const string& alt) const {
    auto i = find(_alt.begin(), _alt.end(), alt);
    if (i == _alt.end())
        return -1;
    return distance(_alt.begin(), i);
}

const CustomValue* Entry::info(const string& key) const {
    auto const& inf = _info.get(*_header);
    auto i = inf.find(key);
    if (i == inf.end())
        return 0;
    return &i->second;
}

bool Entry::isFiltered() const {
    return !_failedFilters.empty()
        && !(_failedFilters.size() == 1 && *_failedFilters.begin() == "PASS");
}

void Entry::setInfo(std::string const& key, CustomValue const& value) {
    auto& inf = _info.get(*_header);
    auto i = inf.find(key);
    if (i == inf.end()) {
        inf.insert(make_pair(key, value));
    } else {
        i->second = std::move(value);
    }
}

SampleData& Entry::sampleData() {
    if (!_parsedSamples) {
        _sampleData.parse(_header, _sampleString);
        _parsedSamples = true;
    }

    return _sampleData;
}

const SampleData& Entry::sampleData() const {
    if (!_parsedSamples) {
        _sampleData.parse(_header, _sampleString);
        _parsedSamples = true;
    }

    return _sampleData;
}

int64_t Entry::start() const {
    return _pos - 1;
    return _startWithoutPadding;
}

int64_t Entry::stop() const {
    return _pos - 1 + _ref.size();
}

int64_t Entry::startWithoutPadding() const {
    return _startWithoutPadding;
}

int64_t Entry::stopWithoutPadding() const {
    return _stopWithoutPadding;
}

const std::string& Entry::alt(GenotypeIndex const& idx) const {
        if (idx == GenotypeIndex::Null)
            return MISSING_STRING;
        else
            return alt()[idx.value];
}

std::vector<std::string> Entry::allelesForSample(size_t sampleIdx) const {
    GenotypeCall const& call = sampleData().genotype(sampleIdx);
    vector<string> rv;
    for (auto i = call.begin(); i != call.end(); ++i) {
        rv.push_back(alt(*i));
    }
    return rv;
}

void Entry::samplesToStream(std::ostream& s) const {
    if (!_parsedSamples) {
        s << _sampleString;
    }
    else {
        s << sampleData();
    }
}

void Entry::allButSamplesToStream(std::ostream& s) const {
    s << _chrom << '\t' << _pos << '\t'
        << streamJoin(identifiers()).delimiter(";").emptyString(".");

    s << '\t' << _ref << '\t'
        << streamJoin(_alt).delimiter(",").emptyString(".");

    if (_qual <= Vcf::Entry::MISSING_QUALITY)
        s << "\t.\t";
    else
        s << '\t' << _qual << '\t';

    s << streamJoin(_failedFilters).delimiter(";").emptyString(".");
    s << '\t' << _info;
}

void Entry::replaceAlts(uint64_t pos, std::string ref, std::vector<std::string> alt) {
    assert(alt.size() == _alt.size());

    _pos = pos;
    _ref = std::move(ref);
    _alt = std::move(alt);
}

void Entry::computeStartStop() {
    // FIXME: don't copy alleles.
    std::vector<std::string> alleles(_alt.size() + 1);
    alleles[0] = _ref;
    std::copy(_alt.begin(), _alt.end(), alleles.begin() + 1);
    _startWithoutPadding = _pos - 1 + commonPrefixMulti(alleles);
    _stopWithoutPadding = _startWithoutPadding + _ref.size() - commonSuffixMulti(alleles);
}

ostream& operator<<(ostream& s, const Entry& e) {
    e.allButSamplesToStream(s);
    s << '\t';
    e.samplesToStream(s);

    return s;
}

END_NAMESPACE(Vcf)
