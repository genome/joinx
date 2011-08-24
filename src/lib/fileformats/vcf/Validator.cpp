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
    , _filters(_header.category("FILTER"))
    , _format(_header.category("FORMAT"))
{
    sort(_info.begin(), _info.end(), mapIdLessThan);
    sort(_filters.begin(), _filters.end(), mapIdLessThan);
    sort(_format.begin(), _format.end(), mapIdLessThan);
}

bool Validator::operator()(const Entry& e, std::vector<std::string>* problems) const {

    bool valid = true;


    //check info fields
    auto info_fields = e.info();
    for(auto it=info_fields.begin();it!=info_fields.end();it++){
        std::string value = (*it).id();
        auto iter = lower_bound(_info.begin(), _info.end(), value, mapIdLessThanString);
        if ((*iter)["ID"] != value) {
            valid = false;
            if (problems) {
                problems->push_back( "Unknown field in info section: \'" + (*it).id() + "\'" );
            }
        }
    }
    auto filters_fields = e.failedFilters();
    for(auto it=filters_fields.begin();it!=filters_fields.end();it++){
        std::string value = (*it);
        auto iter = lower_bound(_filters.begin(), _filters.end(), value, mapIdLessThanString);
        if ((*iter)["ID"] != value) {
            valid = false;
            if (problems) {
                problems->push_back( "Unknown field in filter section: \'" + (*it) + "\'" );
            }
        }
    }


    auto format_fields = e.formatDescription();
    for(auto it=format_fields.begin();it!=format_fields.end();it++){
        Tokenizer<char>  t((*it), ':');
        std::string value;
        while( t.extract(value)){
            auto iter = lower_bound(_format.begin(), _format.end(), value, mapIdLessThanString);
            if ((*iter)["ID"] != value) {
                valid = false;
                if (problems) {
                    problems->push_back( "Unknown field in format section: \'" + value + "\'" );
                }
            }
        }
    }

    return valid;
}

VCF_NAMESPACE_END
