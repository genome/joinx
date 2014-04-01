#pragma once

#include "GenotypeCall.hpp"
#include "common/namespaces.hpp"
#include "common/cstdint.hpp"

#include <boost/unordered_map.hpp>

#include <map>
#include <ostream>
#include <string>
#include <vector>

BEGIN_NAMESPACE(Vcf)

class CustomType;
class CustomValue;
class Header;

class SampleData {
public:
    typedef std::vector<CustomValue> ValueVector;
    typedef std::map<uint32_t, ValueVector*> MapType;
    typedef std::vector<CustomType const*> FormatType;
    typedef MapType::const_iterator const_iterator;

    SampleData();
    SampleData(Header const* h, std::string const& raw);
    SampleData(Header const* h, FormatType&& fmt, MapType&& values);
    SampleData(SampleData const& other);
    SampleData(SampleData&& other);
    SampleData& operator=(SampleData const&);
    SampleData& operator=(SampleData&&);

    ~SampleData();

    Header const& header() const;
    void reheader(Header const* newHeader);

    void clear();
    void swap(SampleData& other);
    void addFilter(uint32_t sampleIdx, std::string const& filterName);


    FormatType const& format() const;
    const_iterator begin() const;
    const_iterator end() const;
    MapType::size_type size() const;
    MapType::size_type count(uint32_t idx) const;
    int formatKeyIndex(std::string const& key) const;

    CustomValue const* get(uint32_t sampleIdx, std::string const& key) const;
    std::vector<CustomValue> const* get(uint32_t sampleIdx) const;

    // returns true if GT is the first FORMAT entry
    bool hasGenotypeData() const;
    GenotypeCall const& genotype(uint32_t sampleIdx) const;

    uint32_t samplesWithData() const;
    bool isSampleFiltered(uint32_t idx, std::string* filterName = 0) const;
    int32_t samplesFailedFilter() const;
    int32_t samplesEvaluatedByFilter() const;
    void removeLowDepthGenotypes(uint32_t lowDepth);

    void renumberGT(std::map<size_t, size_t> const& altMap);

    void formatToStream(std::ostream& s) const;
    void sampleToStream(std::ostream& s, size_t sampleIdx) const;

    void parse(Header const* h, std::string const& raw);
protected:
    void appendFormatField(std::string const& key);
    void appendFormatFieldIfNotExists(std::string const& key);

    void freeValues();

protected:
    Header const* _header;
    std::vector<CustomType const*> _format;
    MapType _values;

    mutable boost::unordered_map<std::string, GenotypeCall> _gtCache;
};

std::ostream& operator<<(std::ostream& s, SampleData const& sampleData);

END_NAMESPACE(Vcf)
