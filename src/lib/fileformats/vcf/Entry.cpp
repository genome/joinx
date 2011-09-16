#include "Entry.hpp"
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

VCF_NAMESPACE_BEGIN

const double Entry::MISSING_QUALITY = numeric_limits<double>::min();

Entry::Entry()
    : _header(0)
    , _pos(0)
    , _qual(MISSING_QUALITY)
{
}

Entry::Entry(const Header* h)
    : _header(h)
    , _pos(0)
    , _qual(MISSING_QUALITY)
{
}

Entry::Entry(const Header* h, const string& s)
    : _header(h)
{
    parse(h, s);
}

const Header& Entry::header() const {
    if (!_header)
        throw runtime_error("Attempted to use Vcf Entry with no header!");
    return *_header;
}

void Entry::parse(const Header* h, const string& s) {
    _header = h;

    Tokenizer<char> tok(s, '\t');
    if (!tok.extract(_chrom))
        throw runtime_error("Failed to extract chromosome from vcf entry" + s);
    if (!tok.extract(_pos))
        throw runtime_error("Failed to extract position from vcf entry" + s);

    string tmp;

    // ids
    if (!tok.extract(tmp))
        throw runtime_error("Failed to extract id from vcf entry" + s);
    extractList(_identifiers, tmp);

    // ref alleles
    if (!tok.extract(_ref))
        throw runtime_error("Failed to extract ref alleles from vcf entry" + s);

    // alt alleles
    if (!tok.extract(tmp))
        throw runtime_error("Failed to extract alt alleles from vcf entry" + s);
    extractList(_alt, tmp, ',');

    // phred quality
    string qualstr;
    if (!tok.extract(qualstr))
        throw runtime_error("Failed to extract quality from vcf entry" + s);
    if (qualstr == ".")
        _qual = MISSING_QUALITY;
    else
        _qual = lexical_cast<double>(qualstr);

    // failed filters
    if (!tok.extract(tmp))
        throw runtime_error("Failed to extract filters from vcf entry" + s);
    extractList(_failedFilters, tmp);
    if (_failedFilters.size() == 1 && _failedFilters[0] == "PASS")
        _failedFilters.clear();

    // info entries
    if (!tok.extract(tmp))
        throw runtime_error("Failed to extract info from vcf entry" + s);
    vector<string> infoStrings;
    extractList(infoStrings, tmp);

    // TODO: refactor into function addInfoField(s)
    _info.clear();
    for (auto i = infoStrings.begin(); i != infoStrings.end(); ++i) {
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
    if (tok.extract(tmp)) {
        extractList(_formatDescription, tmp, ':');
        for (auto i = _formatDescription.begin(); i != _formatDescription.end(); ++i) {
            if (!header().formatType(*i))
                throw runtime_error(str(format("Unknown id in FORMAT field: %1%") %*i));
        }

        _genotypeData.clear();
        // per sample formatted data
        while (tok.extract(tmp)) {
            vector<string> data;
            extractList(data, tmp, ':');
            if (data.size() > _formatDescription.size())
                throw runtime_error("More per-sample values than described in format section");

            vector<CustomValue> perSampleValues;
            for (uint32_t i = 0; i < data.size(); ++i) {
                const CustomType* type = header().formatType(_formatDescription[i]);
                perSampleValues.push_back(CustomValue(type, data[i]));
            }
            _genotypeData.push_back(perSampleValues);
        }
    }
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
    _genotypeData.swap(other._genotypeData);
    std::swap(_header, other._header);
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

const CustomValue* Entry::genotypeData(uint32_t sampleIdx, const string& key) const {
    // no data for that sample
    if (sampleIdx >= _genotypeData.size() || _genotypeData[sampleIdx].empty())
        return 0;

    // no info for that format key
    auto i = find(_formatDescription.begin(), _formatDescription.end(), key);
    if (i == _formatDescription.end())
        return 0;

    uint32_t offset = distance(_formatDescription.begin(), i);
    if (offset >= _genotypeData[sampleIdx].size())
        return 0;
    return &_genotypeData[sampleIdx][offset];
}

Entry Entry::merge(const Header* mergedHeader, const Entry* begin, const Entry* end) {
    map<string, uint32_t> alleleMap;
    uint32_t alleleIdx = 0;
    set<string> identifiers;
    set<string> infoFields;
    set<string> failedFilters;
    vector<string> formatFields; // need to retain order of these
    set<string> sampleNames;
    Entry out(mergedHeader);
    out._chrom = begin->_chrom;
    out._pos = begin->_pos;
    out._qual = MISSING_QUALITY; // not sure how to merge qual values yet
    out._ref = begin->_ref;
    out._genotypeData.resize(mergedHeader->sampleNames().size());

    stringstream posDesc;
    posDesc << out._chrom << ":" << out._pos;

    for (auto entry = begin; entry != end; ++entry) {
        // Merge identifiers
        const vector<string>& idents = entry->identifiers();
        copy(idents.begin(), idents.end(), inserter(identifiers, identifiers.begin()));

        // Merge alleles
        const vector<string>& alleles = entry->alt();
        for (auto alt = alleles.begin(); alt != alleles.end(); ++alt) {
            auto inserted = alleleMap.insert(make_pair(*alt, alleleIdx));
            if (inserted.second) {
                ++alleleIdx;
                out._alt.push_back(*alt);
            }
        }

        // Merge filters
        const vector<string>& filters = entry->failedFilters();
        copy(filters.begin(), filters.end(), inserter(failedFilters, failedFilters.begin()));


        // Build set of all info fields present, validating as we go
        const CustomValueMap& info = entry->info();
        for (auto i = info.begin(); i != info.end(); ++i) {
            infoFields.insert(i->first);
            if (!mergedHeader->infoType(i->first)) {
                throw runtime_error(str(format("Invalid info field '%1%' while merging vcf entries at %2%") %i->first %posDesc.str()));
            }
        }

        // Build set of all format fields present, validating as we go
        const vector<string>& fmt = entry->formatDescription();
        for (auto i = fmt.begin(); i != fmt.end(); ++i) {
            if (find(formatFields.begin(), formatFields.end(), *i) == formatFields.end())
                formatFields.push_back(*i);
            if (!mergedHeader->formatType(*i)) {
                throw runtime_error(str(format("Invalid format field '%1%' while merging vcf entries at %2%") %*i %posDesc.str()));
            }
        }

        const vector<string>& samples = entry->header().sampleNames();
        for (auto i = samples.begin(); i != samples.end(); ++i) {
            auto inserted = sampleNames.insert(*i);
            if (!inserted.second)
                throw runtime_error(str(format("Duplicate sample name '%1%' at %2%") %*i %posDesc.str()));
        }
    }

    // set identifiers in output
    copy(identifiers.begin(), identifiers.end(), back_inserter(out._identifiers));
    out._formatDescription.swap(formatFields);

    GenotypeFormatter genotypeFormatter(mergedHeader, alleleMap);
    // now that we know all the format fields, output per-sample data
    const vector<string>& gtFormat = out._formatDescription; // for convenience
    for (auto entry = begin; entry != end; ++entry) {
        const vector< vector<CustomValue> >& samples = entry->genotypeData();
        for (uint32_t sampleIdx = 0; sampleIdx < samples.size(); ++sampleIdx) {
            if (samples[sampleIdx].empty())
                continue;
            const string& sampleName = entry->header().sampleNames()[sampleIdx];
            uint32_t mergedIdx = mergedHeader->sampleIndex(sampleName);
            out._genotypeData[mergedIdx] = genotypeFormatter.process(gtFormat, entry, sampleIdx);
        }
    }

    // TODO: pass this in, rather than constructing default
    MergeStrategy ms;
    ms.setHeader(mergedHeader);
    for (auto i = infoFields.begin(); i != infoFields.end(); ++i) {
        CustomValue v = ms.mergeInfo(*i, begin, end);
        out._info.insert(make_pair(*i, v));
    }

    return out;
}

VCF_NAMESPACE_END

ostream& operator<<(ostream& s, const Vcf::Entry& e) {
    s << e.chrom() << '\t' << e.pos() << '\t';
    e.printList(s, e.identifiers());
    s << '\t' << e.ref() << '\t';
    e.printList(s, e.alt(), ',');
    if (e.qual() == Vcf::Entry::MISSING_QUALITY)
        s << "\t.\t";
    else
        s << '\t' << e.qual() << '\t';

    if (e.failedFilters().empty())
        s << "PASS";
    else
        e.printList(s, e.failedFilters());
    s << '\t';

    const Vcf::Entry::CustomValueMap& info = e.info();
    for (auto i = info.begin(); i != info.end(); ++i) {
        if (i != info.begin())
            s << ';';
        s << i->second.type().id();
        string value = i->second.toString();
        if (!value.empty())
            s << "=" << value;
    }
    s << '\t';

    e.printList(s, e.formatDescription(), ':');

    const vector< vector<Vcf::CustomValue> >& psd = e.genotypeData();
    for (auto i = psd.begin(); i != psd.end(); ++i) {
        s << '\t';
        if (!i->empty()) {
            for (auto j = i->begin(); j != i->end(); ++j) {
                if (j != i->begin())
                    s << ':';
                j->toStream(s);
            }
        } else
            s << ".";
    }
    return s;
}

