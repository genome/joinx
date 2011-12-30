#include "SampleData.hpp"

#include "CustomType.hpp"
#include "CustomValue.hpp"
#include "GenotypeCall.hpp"
#include "Header.hpp"
#include "common/Tokenizer.hpp"

#include <boost/format.hpp>
#include <algorithm>
#include <functional>
#include <iterator>

using boost::format;
using namespace std::placeholders;
using namespace std;

BEGIN_NAMESPACE(Vcf)
namespace {
    bool customTypeIdMatches(string const& id, CustomType const* type) {
        return type && type->id() == id;
    }
}

SampleData::SampleData()
    : _header(0)
{
}

SampleData::SampleData(Header const* h, std::string const& raw)
    : _header(h)
{
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

            auto type = header().formatType(*i);
            if (!type)
                throw runtime_error(str(boost::format("Unknown id in FORMAT field: %1%") %*i));
            _format.push_back(type);
        }
    }

    uint32_t sampleIdx(0);
    while (tok.extract(&beg, &end)) {
        vector<string> data;
        if (end-beg != 1 || *beg != '.') {
            Tokenizer<char>::split(beg, end, ':', back_inserter(data));

            if (data.size() > _format.size())
                throw runtime_error("More per-sample values than described in format section");

            vector<CustomValue>& values = (_values[sampleIdx] = vector<CustomValue>());
            values.resize(data.size());
            for (uint32_t i = 0; i < data.size(); ++i) {
                values[i] = CustomValue(_format[i], data[i]);
            }
        }
        ++sampleIdx;
    }
}

SampleData::SampleData(Header const* h, FormatType&& fmt, MapType&& values)
    : _header(h)
{
    _format.swap(fmt);
    _values.swap(values);
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
        newData[newIdx].swap(i->second);
    }

    _header = newHeader;
    _values.swap(newData);
}

void SampleData::clear() {
    _header = 0;
    _format.clear();
    _values.clear();
}

void SampleData::swap(SampleData& other) {
    std::swap(_header, other._header);
    _format.swap(other._format);
    _values.swap(other._values);
}

SampleData::FormatType const& SampleData::format() const {
    return _format;
}

CustomValue const* SampleData::get(uint32_t sampleIdx, std::string const& key) const {
    // no data for that sample
    auto iter = _values.find(sampleIdx);
    if (iter == _values.end())
        return 0;

    // no info for that format key
    auto i = find_if(_format.begin(), _format.end(), bind(&customTypeIdMatches, key, _1));
    if (i == _format.end())
        return 0;

    uint32_t offset = distance(_format.begin(), i);
    if (offset >= iter->second.size())
        return 0;

    return &iter->second[offset];
}

std::vector<CustomValue> const* SampleData::get(uint32_t sampleIdx) const {
    auto iter = _values.find(sampleIdx);
    if (iter == _values.end())
        return 0;

    return &iter->second;
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

GenotypeCall SampleData::genotype(uint32_t sampleIdx) const {
    const string* gtString(0);
    const CustomValue* v = get(sampleIdx, "GT");
    if (!v || v->empty() || (gtString = v->get<string>(0)) == 0 || gtString->empty())
        return GenotypeCall();
    return GenotypeCall(*gtString);
}

uint32_t SampleData::samplesWithData() const {
    uint32_t rv(0);
    for (auto i = _values.begin(); i != _values.end(); ++i)
        if (!i->second.empty())
            ++rv;
    return rv;
}

int32_t SampleData::samplesFailedFilter() const {
    auto i = find_if(_format.begin(), _format.end(), bind(&customTypeIdMatches, "FT", _1));
    if (i == _format.end())
        return -1;

    uint32_t offset = distance(_format.begin(), i);
    uint32_t numFailedFilter = 0;
    for (auto i = _values.begin(); i != _values.end(); ++i) {
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

int32_t SampleData::samplesEvaluatedByFilter() const {
    auto i = find_if(_format.begin(), _format.end(), bind(&customTypeIdMatches, "FT", _1));
    if (i == _format.end())
        return -1;

    uint32_t offset = distance(_format.begin(), i);
    uint32_t numEvaluatedByFilter = 0;
    for (auto i = _values.begin(); i != _values.end(); ++i) {
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

void SampleData::removeLowDepthGenotypes(uint32_t lowDepth) {
    auto i = find_if(_format.begin(), _format.end(), bind(&customTypeIdMatches, "DP", _1));
    if (i == _format.end())
        return;

    uint32_t offset = distance(_format.begin(), i);
    for (auto i = _values.begin(); i != _values.end(); ++i) {
        const int64_t *v;
        if (i->second[offset].empty() || (v = i->second[offset].get<int64_t>(0)) == NULL || *v < lowDepth)
            i->second.clear();
    }

}

END_NAMESPACE(Vcf)

std::ostream& operator<<(std::ostream& s, Vcf::SampleData const& sampleData) {
    auto const& fmt = sampleData.format();
    if (!fmt.empty()) {
        for (auto i = fmt.begin(); i != fmt.end(); ++i) {
            if (i != fmt.begin())
                s << ':';
            s << (*i)->id();
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
