#pragma once

#include "common/Tokenizer.hpp"
#include "namespace.hpp"

#include <boost/lexical_cast.hpp>
#include <cstdint>
#include <ostream>
#include <string>
#include <vector>

VCF_NAMESPACE_BEGIN

class Header;

class InfoField {
public:
    InfoField();
    InfoField(const std::string& raw);
    InfoField(const std::string& id, const std::string& val);

    const std::string& id() const;
    const std::string& value() const;

    template<typename T>
    T as() const { return boost::lexical_cast<T>(_value); }

    bool operator==(const InfoField& rhs) const {
        return _id == rhs._id && _value == rhs._value;
    }

protected:
    std::string _id;
    std::string _value;
};

class Entry {
public:
    template<typename T>
    static void parseLine2(const T& reader, const std::string& s, Entry& e) {
        e.parse2(reader, s);
    }

    static void parseLine(const std::string& s, Entry& e) {
        return e.parse(s);
    }

    Entry();
    explicit Entry(const std::string& s);
    Entry(
        // non variable length fields
        const std::string& chrom,
        uint64_t pos,
        const std::string& ref,
        double qual
        );

    void addIdentifier(const std::string& id);
    void addAlt(const std::string& alt);
    void addFailedFilter(const std::string& filter);
    void addInfoField(const std::string& key, const std::string& value);
    void addFormatDescription(const std::string& desc);
    void addPerSampleData(const std::string& key, const std::string& value);

    void parse(const std::string& s);
    void parse2(const Header& h, const std::string& s);

    const std::string& line() const { return _line; }
    const std::string& chrom() const { return _chrom; }
    uint64_t pos() const { return _pos; }
    const std::vector<std::string>& identifiers() const { return _identifiers; }
    const std::string& ref() const { return _ref; }
    const std::vector<std::string>& alt() const { return _alt; }
    double qual() const { return _qual; }
    const std::vector<std::string>& failedFilters() const { return _failedFilters; }
    const std::vector<InfoField>& info() const { return _info; }
    const std::vector<std::string>& formatDescription() const { return _formatDescription; }
    const std::vector< std::vector<std::string> >& perSampleData() const { return _perSampleData; }

    std::string toString() const;

    template<typename T>
    void extractList(T& v, const std::string& s, char delim = ';') {
        v.clear();
        if (s == ".")
            return;

        Tokenizer<char> t(s, delim);
        typename T::value_type tmp;
        while (t.extract(tmp)) {
            v.push_back(tmp);
        }
    }

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

protected:
    std::string _line;
    std::string _chrom;
    uint64_t _pos;
    std::vector<std::string> _identifiers;
    std::string _ref;
    std::vector<std::string> _alt;
    double _qual;
    std::vector<std::string> _failedFilters;
    std::vector<InfoField> _info;
    std::vector<std::string> _formatDescription;
    std::vector< std::vector<std::string> > _perSampleData;
};

inline bool Entry::operator<(const Entry& rhs) const {
    return cmp(rhs) < 0;
}

VCF_NAMESPACE_END

std::ostream& operator<<(std::ostream& s, const Vcf::InfoField& i);
std::ostream& operator<<(std::ostream& s, const Vcf::Entry& e);
