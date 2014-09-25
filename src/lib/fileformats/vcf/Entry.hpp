#pragma once

#include "CustomValue.hpp"
#include "InfoFields.hpp"
#include "SampleData.hpp"
#include "common/CoordinateView.hpp"
#include "common/LocusCompare.hpp"
#include "common/Tokenizer.hpp"
#include "common/cstdint.hpp"
#include "common/namespaces.hpp"

#include <boost/lexical_cast.hpp>
#include <map>
#include <ostream>
#include <string>
#include <vector>
#include <set>

//TODO needs addFilter function. Make sure to check if filter is available in header
BEGIN_NAMESPACE(Vcf)

class EntryMerger;
class Header;

class Entry {
public:
    typedef LocusCompare<DefaultCoordinateView, StartOnly> DefaultCompare;

    static bool isInvalidFilterId (char c) {
        return (c == ';' || isspace(c));
    }

    enum FieldName {
        CHROM,
        POS,
        ID,
        REF,
        ALT,
        QUAL,
        FILTER,
        INFO,
        FORMAT,
        SAMPLE_DATA,
        UNDEFINED
    };

    typedef Header HeaderType;
    typedef std::map<std::string, CustomValue> CustomValueMap;

    // static data
    static const double MISSING_QUALITY;

    // static functions
    static const char* fieldToString(FieldName field);
    static FieldName fieldFromString(const char* name);
    static void parseLine(const Header* hdr, std::string& s, Entry& e);
    static void parseLineAndReheader(const Header* hdr, const Header* newH, std::string& s, Entry& e);
    static bool posLess(Entry const& a, Entry const& b);
    static bool chromEq(const std::string& chrom, Entry const& b);


    // member functions
    Entry();
    Entry(Entry const& e);
    // for move semantics
    Entry(Entry&& e);
    explicit Entry(const Header* h);
    Entry(EntryMerger&& merger);
    Entry(const Header* h, const std::string& s);

    Entry& operator=(Entry const& e);
    Entry& operator=(Entry&& e);

    // This updates the entry to use the new header instead. This can
    // move the sample data around, but does not currently do any more
    // validation.
    void reheader(const Header* newHeader);

    const Header& header() const;
    void parse(const Header* h, const std::string& s);
    void parseAndReheader(const Header* h, const Header* newH, const std::string& s);

    void addIdentifier(const std::string& id);
    void addFilter(const std::string& filterName);
    void clearFilters();

    const std::string& chrom() const { return _chrom; }
    const uint64_t& pos() const { return _pos; }
    const std::set<std::string>& identifiers() const { return _identifiers; }
    const std::string& ref() const { return _ref; }
    const std::vector<std::string>& alt() const { return _alt; }
    const std::string& alt(GenotypeIndex const& idx) const;
    double qual() const { return _qual; }
    const std::set<std::string>& failedFilters() const { return _failedFilters; }
    const CustomValueMap& info() const { return _info.get(header()); }
    const CustomValue* info(std::string const& key) const;
    void setInfo(std::string const& key, CustomValue const& value);
    const SampleData& sampleData() const;
    SampleData& sampleData();

    bool isFiltered() const;

    template<typename T>
    bool isFilteredByAnythingExcept(T const& whitelist) {
        // Do the quick check first
        if (!isFiltered()) {
            return false;
        }

        auto const& filters = failedFilters();
        for (auto i = filters.begin(); i != filters.end(); ++i) {
            if (*i != "PASS" && whitelist.count(*i) == 0) {
                return true;
            }
        }
        return false;
    }

    int64_t start() const;
    int64_t stop() const;
    int64_t startWithoutPadding() const;
    int64_t stopWithoutPadding() const;

    int64_t length() const { return stop() - start(); }

    // -1 if not found
    int32_t altIdx(const std::string& alt) const;

    std::string toString() const;
    std::vector<std::string> allelesForSample(size_t sampleIdx) const;

    void swap(Entry& other);

    void allButSamplesToStream(std::ostream& s) const;
    void samplesToStream(std::ostream& s) const;

    void replaceAlts(uint64_t pos, std::string ref, std::vector<std::string> alt);
    void computeStartStop();

protected:
    const Header* _header;
    std::string _chrom;
    uint64_t _pos;
    int64_t _startWithoutPadding;
    int64_t _stopWithoutPadding;
    std::set<std::string> _identifiers;
    std::string _ref;
    std::vector<std::string> _alt;
    double _qual;
    std::set<std::string> _failedFilters;
    InfoFields _info;
    std::string _sampleString;
    mutable bool _parsedSamples;
    mutable SampleData _sampleData;
};

inline bool containsInsertions(Vcf::Entry const& v) {
    // no lambdas in gcc 4.4 :(
    for (auto i = v.alt().begin(); i != v.alt().end(); ++i) {
        if (i->size() > v.ref().size())
            return true;
    }
    return false;
}

std::ostream& operator<<(std::ostream& s, const Entry& e);

END_NAMESPACE(Vcf)

