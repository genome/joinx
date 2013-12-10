#pragma once

#include "CustomValue.hpp"
#include "SampleData.hpp"
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
    friend class AltNormalizer;

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
    Entry(Entry const& e) throw();
    // for move semantics
    Entry(Entry&& e) throw();
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
    double qual() const { return _qual; }
    const std::set<std::string>& failedFilters() const { return _failedFilters; }
    const CustomValueMap& info() const { return _info; }
    bool isFiltered() const;
    const CustomValue* info(std::string const& key) const;
    void setInfo(std::string const& key, CustomValue const& value);
    const SampleData& sampleData() const;
    SampleData& sampleData();

    int64_t start() const;
    int64_t stop() const;
    int64_t length() const { return stop() - start(); }

    // -1 if not found
    int32_t altIdx(const std::string& alt) const;

    std::string toString() const;
    std::vector<std::string> allelesForSample(size_t sampleIdx) const;

    template<typename T>
    void printList(std::ostream& s, const T& v, char delim = ';') const {
        if (!v.empty()) {
            for (auto i = v.begin(); i != v.end(); ++i) {
                if (i != v.begin())
                    s << delim;
                s << *i;
            }
        } else {
            s << '.';
        }
    }

    int cmp(const Entry& rhs) const;
    bool operator<(const Entry& rhs) const;

    void swap(Entry& other);

    void samplesToStream(std::ostream& s) const;

protected:
    const Header* _header;
    std::string _chrom;
    uint64_t _pos;
    std::set<std::string> _identifiers;
    std::string _ref;
    std::vector<std::string> _alt;
    double _qual;
    std::set<std::string> _failedFilters;
    CustomValueMap _info;
    std::string _sampleString;
    mutable bool _parsedSamples;
    mutable SampleData _sampleData;
};

inline bool Entry::operator<(const Entry& rhs) const {
    return cmp(rhs) < 0;
}

std::ostream& operator<<(std::ostream& s, const Entry& e);

END_NAMESPACE(Vcf)

