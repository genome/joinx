#include "Validator.hpp"
#include "Entry.hpp"

#include <algorithm>

using namespace std;

VCF_NAMESPACE_BEGIN

namespace {
    // utility functions to compare Vcf::Map objects based on the ID key
    // note: these will throw if the ID key is missing
    bool mapIdLessThan(const Map& a, const Map& b) {
        return a["ID"] < b["ID"];
    }

    bool mapIdEquals(const Map& a, const Map& b) {
        return a["ID"] == b["ID"];
    }

    // This are to enable lower_bould using strings
    bool mapIdLessThanString(const Map& a, const string& b) {
        return a["ID"] < b;
    }
}

Validator::Validator(const Header& header)
    : _header(header)
    , _info(_header.category("INFO"))
    , _filters(_header.category("FILTERS"))
    , _format(_header.category("FORMAT"))
{
    sort(_info.begin(), _info.end(), mapIdLessThan);
    sort(_filters.begin(), _filters.end(), mapIdLessThan);
    sort(_format.begin(), _format.end(), mapIdLessThan);
}

bool Validator::operator()(const Entry& e, std::vector<std::string>* problems) const {
    // TODO: loop over all the data in the entry that requires definitions in the header
    // Here is an example of how to check if the field "DP" exists in the _info section:
    bool valid = true;
    string value = "DP";
    auto iter = lower_bound(_info.begin(), _info.end(), value, mapIdLessThanString);
    if ((*iter)["ID"] != value) {
        // BOOM
    }
    // TODO: could go even farther and validate the data is valid for (*iter)["Type"], etc

    if (problems) {
        problems->push_back("troll problem");
    }
    return valid;
}

VCF_NAMESPACE_END
