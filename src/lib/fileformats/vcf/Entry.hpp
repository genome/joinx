#pragma once

#include "common/Tokenizer.hpp"
#include "namespace.hpp"

#include <boost/lexical_cast.hpp>
#include <cstdint>
#include <ostream>
#include <string>
#include <vector>

VCF_NAMESPACE_BEGIN

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
    Entry();
    explicit Entry(const std::string& s);

    void parse(const std::string& s);

    const std::string& line() const { return _line; }
    const std::string& chrom() const { return _chrom; }
    uint64_t pos() const { return _pos; }
    const std::vector<std::string>& identifiers() const { return _identifiers; }
    const std::string& ref() const { return _ref; }
    const std::vector<std::string>& alt() const { return _alt; }
    float qual() const { return _qual; }
    const std::vector<std::string>& failedFilters() const { return _failedFilters; }
    const std::vector<InfoField>& info() const { return _info; }
    const std::vector<std::string>& formatDescription() const { return _formatDescription; }
    const std::vector< std::vector<std::string> >& perSampleData() const { return _perSampleData; }

    std::string toString() const;

    template<typename T>
    void extractList(T& v, const std::string& s, char delim = ';') {
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

protected:
    std::string _line;
    std::string _chrom;
    uint64_t _pos;
    std::vector<std::string> _identifiers;
    std::string _ref;
    std::vector<std::string> _alt;
    float _qual;
    std::vector<std::string> _failedFilters;
    std::vector<InfoField> _info;
    std::vector<std::string> _formatDescription;
    std::vector< std::vector<std::string> > _perSampleData;
};

VCF_NAMESPACE_END

std::ostream& operator<<(std::ostream& s, const Vcf::InfoField& i);
std::ostream& operator<<(std::ostream& s, const Vcf::Entry& e);
