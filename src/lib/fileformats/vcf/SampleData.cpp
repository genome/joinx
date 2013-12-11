#include "SampleData.hpp"

#include "CustomType.hpp"
#include "CustomValue.hpp"
#include "GenotypeCall.hpp"
#include "Header.hpp"
#include "common/Tokenizer.hpp"
#include "io/StreamJoin.hpp"

#include <boost/bind.hpp>
#include <boost/format.hpp>

#include <algorithm>
#include <functional>
#include <iterator>
#include <iterator>
#include <memory>
#include <set>
#include <sstream>
#include <utility>

using boost::format;
using namespace std;

BEGIN_NAMESPACE(Vcf)
namespace {
    bool customTypeIdMatches(string const& id, CustomType const* type) {
        return type && type->id() == id;
    }
}

SampleData& SampleData::operator=(SampleData const& other) {
    freeValues();
    _header = other._header;
    _format = other._format;
    // deep copy values
    for (auto i = other._values.begin(); i != other._values.end(); ++i)
        _values.insert(_values.end(), make_pair(i->first, new ValueVector(*i->second)));
    return *this;
}



SampleData::SampleData()
    : _header(0)
{
}

SampleData::SampleData(SampleData const& other) {
    *this = other;
}

SampleData::SampleData(SampleData&& other)
    : _header(other._header)
    , _format(std::move(other._format))
    , _values(std::move(other._values))
{
}

SampleData::SampleData(Header const* h, std::string const& raw)
    : _header(h)
{
    try {
        parse(raw);
    } catch (...) {
        freeValues();
        throw;
    }
}

void SampleData::parse(std::string const& raw) {
    Tokenizer<char> tok(raw, '\t');
    char const* beg(0);
    char const* end(0);

    vector<string> fmt;
    if (tok.extract(&beg, &end) && (end - beg != 1 || *beg != '.')) {
        Tokenizer<char>::split(beg, end, ':', back_inserter(fmt));

        _format.reserve(fmt.size());
        for (auto i = fmt.begin(); i != fmt.end(); ++i) {
            if (i->empty())
                continue;

            appendFormatField(*i);
        }
    }

    uint32_t sampleIdx(0);
    while (tok.extract(&beg, &end)) {
        vector<string> data;

        // allow trailing tabs because our data has some :/
        if (tok.eof() && end-beg == 0)
            break;

        if (end-beg != 1 || *beg != '.') {
            Tokenizer<char>::split(beg, end, ':', back_inserter(data));

            if (data.size() > _format.size())
                throw runtime_error("More per-sample values than described in format section");

            std::auto_ptr<ValueVector> values(new ValueVector);
            values->resize(data.size());
            for (uint32_t i = 0; i < data.size(); ++i) {
                (*values)[i] = CustomValue(_format[i], data[i]);
            }
            _values.insert(make_pair(sampleIdx, values.release()));
        }
        ++sampleIdx;
    }

    auto const& mirrored = _header->mirroredSamples();
    for (auto i = mirrored.begin(); i != mirrored.end(); ++i) {
        size_t targetIdx = i->first;
        size_t srcIdx = i->second;

        auto src = _values.find(srcIdx);
        if (src != _values.end()) {
            auto inserted = _values.insert(make_pair(targetIdx, src->second));
            if (!inserted.second)
                throw runtime_error("Internal error: column mirroring.");
        }
    }


    if (sampleIdx > _header->sampleNames().size()) {
        throw runtime_error(str(boost::format("More samples than described in VCF header (%1% vs %2%).") %sampleIdx %_header->sampleNames().size()));
    }
}

SampleData::SampleData(Header const* h, FormatType&& fmt, MapType&& values)
    : _header(h)
{
    _format.swap(fmt);
    _values.swap(values);
}

SampleData::~SampleData() {
    freeValues();
}

void SampleData::freeValues() {
    // Mirrored columns can lead to multiple copies of the same
    // pointer appearing in the _values map. We don't want to
    // delete them twice.
    set<ValueVector*> uniqPtrs;
    for (auto i = _values.begin(); i != _values.end(); ++i) {
        auto seen = uniqPtrs.insert(i->second);
        if (seen.second) {
            delete i->second;
        }
    }
    _values.clear();
}

Header const& SampleData::header() const {
    if (!_header)
        throw runtime_error("Attempted to use Vcf SampleData with no header!");

    return *_header;
}

void SampleData::reheader(Header const* newHeader) {
    if (!newHeader)
        throw runtime_error("Attempted to reheader Vcf SampleData with null header!");

    MapType newData;
    for (auto i = _values.begin(); i != _values.end(); ++i) {
        const string& sampleName = header().sampleNames()[i->first];
        uint32_t newIdx = newHeader->sampleIndex(sampleName);
        std::swap(newData[newIdx], i->second);
    }

    _header = newHeader;
    _values.swap(newData);
}

void SampleData::clear() {
    _header = 0;
    _format.clear();
    freeValues();
}

void SampleData::swap(SampleData& other) {
    std::swap(_header, other._header);
    _format.swap(other._format);
    _values.swap(other._values);
}

void SampleData::addFilter(uint32_t sampleIdx, std::string const& filterName) {
    auto sampleIter = _values.find(sampleIdx);
    if (sampleIter == _values.end() || sampleIter->second == 0) {
        cerr << "Warning: attempted to filter nonexistant sample\n";
        return;
    }

    CustomType const* FT = _header->formatType("FT");
    if (FT == 0) {
        throw runtime_error(
            str(boost::format("Attempted to filter sample %1% with filter %2%, but "
                "no FT FORMAT tag appears in header") %sampleIdx %filterName));
    }

    auto ftIter = find_if(_format.begin(), _format.end(),
            boost::bind(&customTypeIdMatches, "FT", _1));

    size_t ftIdx(0);
    if (ftIter == _format.end()) {
        _format.push_back(FT);
        ftIdx = _format.size()-1;
    } else {
        ftIdx = ftIter - _format.begin();
    }


    if (sampleIter->second->size() <= ftIdx) {
        sampleIter->second->resize(ftIdx+1);
    }

    ValueVector& values = *sampleIter->second;
    auto& prev = values[ftIdx];
    set<string> filters;
    if (!prev.empty())
        Tokenizer<char>::split(prev.toString(), ';', inserter(filters, filters.begin()));

    filters.erase(".");
    filters.erase("PASS");
    filters.insert(filterName);
    stringstream ss;
    ss << streamJoin(filters).delimiter(";").emptyString(".");
    values[ftIdx] = CustomValue(FT, ss.str());
}

SampleData::FormatType const& SampleData::format() const {
    return _format;
}

int SampleData::formatKeyIndex(std::string const& key) const {
    auto i = find_if(_format.begin(), _format.end(),
            boost::bind(&customTypeIdMatches, key, _1));
    if (i == _format.end())
        return -1;
    return distance(_format.begin(), i);
}

CustomValue const* SampleData::get(uint32_t sampleIdx, std::string const& key) const {
    ValueVector const* values = get(sampleIdx);

    // no data for that sample
    if (!values)
        return 0;

    // no info for that format key
    int offset = formatKeyIndex(key);
    if (offset == -1)
        return 0;

    if (size_t(offset) >= values->size())
        return 0;

    return &(*values)[offset];
}

SampleData::ValueVector const* SampleData::get(uint32_t sampleIdx) const {
    auto iter = _values.find(sampleIdx);
    if (iter == _values.end())
        return 0;

    return iter->second;
}

SampleData::iterator SampleData::begin() {
    return _values.begin();
}

SampleData::iterator SampleData::end() {
    return _values.end();
}

SampleData::const_iterator SampleData::begin() const {
    return _values.begin();
}

SampleData::const_iterator SampleData::end() const {
    return _values.end();
}

SampleData::MapType::size_type SampleData::size() const {
    return _values.size();
}

SampleData::MapType::size_type SampleData::count(uint32_t idx) const {
    return _values.count(idx);
}

bool SampleData::hasGenotypeData() const {
    return !_format.empty() && _format.front()->id() == "GT";
}

GenotypeCall const& SampleData::genotype(uint32_t sampleIdx) const {
    const string* gtString(0);
    const CustomValue* v = get(sampleIdx, "GT");
    if (!v || v->empty() || (gtString = v->get<string>(0)) == 0 || gtString->empty())
        return GenotypeCall::Null;

    auto inserted = _gtCache.insert(make_pair(*gtString, GenotypeCall()));
    // if it wasn't already in the cache
    if (inserted.second)
        inserted.first->second = GenotypeCall(*gtString);
    return inserted.first->second;
}

uint32_t SampleData::samplesWithData() const {
    uint32_t rv(0);
    for (auto i = _values.begin(); i != _values.end(); ++i)
        if (!i->second->empty())
            ++rv;
    return rv;
}

bool SampleData::isSampleFiltered(uint32_t idx, std::string* filterName) const {
    filterName = 0;
    CustomValue const* ft = get(idx, "FT");
    if (!ft || ft->empty())
        return false;

    for (size_t i = 0; i < ft->size(); ++i) {
        string const& f = ft->getString(i);
        if (!f.empty() && f != "." && f != "PASS") {
            if (filterName)
                *filterName = f;
            return true;
        }
    }
    return false;
}

int32_t SampleData::samplesFailedFilter() const {
    auto i = find_if(_format.begin(), _format.end(),
            boost::bind(&customTypeIdMatches, "FT", _1));

    if (i == _format.end())
        return -1;

    uint32_t offset = distance(_format.begin(), i);
    uint32_t numFailedFilter = 0;
    for (auto i = _values.begin(); i != _values.end(); ++i) {
        if (i->second == 0)
            continue;
        auto const& values = *i->second;
        if (values.size() > offset) {
            //then we have some data
            const std::string *filter;
            //if it has a value (assume . is processed correctly) and we're able to get a value and it is not pass then failed
            if (!values[offset].empty() && (filter = values[offset].get<std::string>(0)) != 0 && *filter != std::string("PASS")) {
               numFailedFilter++;
            }
        }
    }
    return numFailedFilter;
}

int32_t SampleData::samplesEvaluatedByFilter() const {
    auto i = find_if(_format.begin(), _format.end(),
            boost::bind(&customTypeIdMatches, "FT", _1));

    if (i == _format.end())
        return -1;

    uint32_t offset = distance(_format.begin(), i);
    uint32_t numEvaluatedByFilter = 0;
    for (auto i = _values.begin(); i != _values.end(); ++i) {
        if (i->second == 0)
            continue;
        auto const& values = *i->second;
        if (values.size() > offset) {
            //then we have some data
            //if it has a value (assume . is processed correctly) and we're able to get a value and it is not pass then failed
            if (!values[offset].empty() && values[offset].get<std::string>(0) != 0) {
               numEvaluatedByFilter++;
            }
        }
    }
    return numEvaluatedByFilter;
}

void SampleData::renumberGT(std::map<size_t, size_t> const& altMap) {
    int gtIdx = formatKeyIndex("GT");
    if (gtIdx == -1)
        return;

    for (auto iter = _values.begin(); iter != _values.end(); ++iter) {
        ValueVector& vals = *iter->second;
        if (vals.size() <= size_t(gtIdx))
            continue;
        CustomValue& gt = vals[gtIdx];
        string const* gtStr = gt.get<string>();
        if (gtStr == 0)
            continue;

        GenotypeCall old(*gtStr);
        char delim = old.phased() ? '|' : '/';
        stringstream newss;
        for (auto alt = old.begin(); alt != old.end(); ++alt) {
            if (alt != old.begin())
                newss << delim;
            auto remapped = altMap.find(*alt);
            if (remapped == altMap.end()) {
                newss << *alt;
            } else {
                newss << remapped->second;
            }
        }
        vals[gtIdx].set(0, newss.str());
    }
}

void SampleData::removeLowDepthGenotypes(uint32_t lowDepth) {
    auto i = find_if(_format.begin(), _format.end(),
            boost::bind(&customTypeIdMatches, "DP", _1));

    if (i == _format.end())
        return;

    uint32_t offset = distance(_format.begin(), i);
    for (auto i = _values.begin(); i != _values.end(); ++i) {
        if (i->second == 0)
            continue;
        auto& values = *i->second;
        const int64_t *v;
        if (values[offset].empty() || (v = values[offset].get<int64_t>(0)) == 0 || *v < lowDepth)
            values.clear();
    }
}

void SampleData::sampleToStream(std::ostream& s, size_t sampleIdx) const {
    auto data = get(sampleIdx);
    if (!data) {
        s << ".";
    }
    else {
        s << streamJoin(*data).delimiter(":");
    }
}

void SampleData::appendFormatFieldIfNotExists(std::string const& key) {
    if (formatKeyIndex(key) == -1) {
        appendFormatField(key);
    }
}

void SampleData::appendFormatField(std::string const& key) {
    auto type = header().formatType(key);
    if (!type) {
        throw runtime_error(str(boost::format("Unknown id in FORMAT field: %1%") % key));
    }
    _format.push_back(type);
}

std::ostream& operator<<(std::ostream& s, SampleData const& sampleData) {
    auto const& fmt = sampleData.format();
    if (!fmt.empty()) {
        auto i = fmt.begin();
        s << (*i)->id();
        for (++i; i != fmt.end(); ++i) {
            s << ':' << (*i)->id();
        }
    } else {
        s << '.';
    }

    uint32_t sampleCounter(0);
    uint32_t nSamples = sampleData.header().sampleCount();
    for (auto i = sampleData.begin(); i != sampleData.end(); ++i) {
        s << '\t';
        while (sampleCounter < i->first) {
            s << ".\t";
            ++sampleCounter;
        }

        if (i->second == 0)
            continue;
        auto const& values = *i->second;
        s << streamJoin(values).delimiter(":").emptyString(".");
        ++sampleCounter;
    }

    while (sampleCounter++ < nSamples) {
        s << "\t.";
    }


    return s;
}

END_NAMESPACE(Vcf)
