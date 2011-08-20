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
 
    //populate _*_list's with fields defined in the header
    for(auto it = _info.begin(); it != _info.end(); it++){
        _info_list.insert((*it)["ID"]);
    }
    for(auto it = _filters.begin(); it != _filters.end(); it++){
        _filters_list.insert((*it)["ID"]);
    }
    for(auto it = _format.begin(); it != _format.end(); it++){
        _format_list.insert( (*it)["ID"] );
    }
}

bool Validator::operator()(const Entry& e, std::vector<std::string>* problems) const {
    // TODO: loop over all the data in the entry that requires definitions in the header
    // Here is an example of how to check if the field "DP" exists in the _info section:

    bool valid = true;

    //check info fields
    auto info_fields = e.info();
    for(auto it=info_fields.begin();it!=info_fields.end();it++){
        if(! _info_list.count((*it).id()) ) {
            valid = false;
            if (problems) {
                problems->push_back( "Unknown field in info section: \'" + (*it).id() + "\'" );
            }
        }
    }

    //check filter fields
    auto filters_fields = e.failedFilters();
    for(auto it=filters_fields.begin();it!=filters_fields.end();it++){
        if(! _filters_list.count((*it)) ) {
            valid = false;
            if (problems) {
                problems->push_back( "Unknown field in filter section: \'" + (*it) + "\'" );
            }
        }
    }

    //check format fields
    auto format_fields = e.formatDescription();
    for(auto it=format_fields.begin();it!=format_fields.end();it++){
        Tokenizer<char>  t((*it), ':');
        std::string field;
        while( t.extract(field)){
            if(! _format_list.count( field )) {
                valid = false; 
                if (problems) {
                    problems->push_back( "Unknown field in format section: \'" + field + "\'" );
                }
            }
        }
    }

    return valid;
}

VCF_NAMESPACE_END
