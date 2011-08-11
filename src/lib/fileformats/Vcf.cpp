#include "Vcf.hpp"

#include <boost/format.hpp>
#include <algorithm>
#include <functional>
#include <sstream>
#include <stdexcept>

using namespace std;
using namespace std::placeholders;
using boost::format;

VCF_NAMESPACE_BEGIN

namespace {
    struct StrType {
        string txt;
        DataType type;

        bool operator<(const StrType& rhs) const { return txt < rhs.txt; }
        bool operator<(const string& rhs) const { return txt < rhs; }
        bool operator==(const StrType& rhs) const { return txt == rhs.txt; }
        bool operator==(const string& rhs) const { return txt == rhs; }
    } __str_types[] = {
        {"Character", CHAR},
        {"Flag", FLAG},
        {"Float", FLOAT},
        {"Integer", INT},
        {"String", STRING},
    };

    const uint32_t __n_types = sizeof(__str_types)/sizeof(__str_types[0]);
}


DataType strToType(const string& s) {
    StrType* p = lower_bound(__str_types, __str_types+__n_types, s);
    if (p != __str_types+__n_types && p->txt == s)
        return p->type;
    throw runtime_error(str(format("Unknown data type '%1%'") %s));
}

string parseString(const string& s) {
    if (s.size() < 2)
        throw runtime_error(str(format("Invalid vcf string: %1%") %s));

    if (s[0] != '"' || s[s.size()-1] != '"')
        throw runtime_error(str(format("Unquoted vcf string: %1%") %s));

    string tmp = s.substr(1, s.size()-2);

    string::size_type lastPos = 0;
    stringstream ss;
    string::size_type pos;
    do {
        pos = tmp.find('\\', lastPos);
        if (pos == tmp.size() - 1)
            throw runtime_error(str(format("Trailing backslash in vcf string: %1%") %s));
        ss << tmp.substr(lastPos, pos-lastPos);
        if (pos != string::npos)
            ss << tmp[pos+1];
        lastPos = pos+2;
        string curr = ss.str();
        string rest = tmp.substr(lastPos);
    } while (pos < tmp.size());
    return ss.str();
}


VCF_NAMESPACE_END
