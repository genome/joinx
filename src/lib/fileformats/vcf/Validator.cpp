#include "Validator.hpp"
#include "Entry.hpp"
#include <stdlib.h>
#include <algorithm>
#include <boost/lexical_cast.hpp>

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
        std::string id = (*it).id();
        auto iter = lower_bound(_info.begin(), _info.end(), id, mapIdLessThanString);
        if ((*iter)["ID"] != id) {
            valid = false;
            if (problems) {
                problems->push_back( "Unknown field in info section: \'" + (*it).id() + "\'" );
            }
            continue;
        }
        valid = typeCheck( (*it).value(), (*iter)["Type"], problems) && valid;
    }

    //check filter fields
    auto filters_fields = e.failedFilters();
    for(auto it=filters_fields.begin();it!=filters_fields.end();it++){
        std::string id = (*it);
        auto iter = lower_bound(_filters.begin(), _filters.end(), id, mapIdLessThanString);
        if ((*iter)["ID"] != id) {
            valid = false;
            if (problems) {
                problems->push_back( "Unknown field in filter section: \'" + (*it) + "\'" );
            }
            continue;
        }
    }

    //check format fields
    std::vector<std::string> format_types;
    auto format_fields = e.formatDescription();
    for(auto it=format_fields.begin();it!=format_fields.end();it++){
        std::string id = (*it);
        auto iter = lower_bound(_format.begin(), _format.end(), id, mapIdLessThanString);
        if ((*iter)["ID"] != id) {
            valid = false;
            if (problems) {
                problems->push_back( "Unknown field in format section: \'" + id + "\'" );
            }
            continue;
        }
        format_types.push_back((*iter)["Type"]);
    }
    //check per-sample format fields
    auto perSampleLines = e.perSampleData();
    for (auto perSampleIter = perSampleLines.begin(); perSampleIter != perSampleLines.end(); perSampleIter++){
        if((*perSampleIter).size() != format_types.size()){
            valid = false;
            if( problems ){
                problems->push_back( "Not enough genotype fields." );
            }
            continue;
        }
        int field_index = 0;
        for (auto perSampleFieldIter = (*perSampleIter).begin(); perSampleFieldIter != (*perSampleIter).end(); perSampleFieldIter++){
            valid = typeCheck( (*perSampleFieldIter), (*(format_types.begin()+field_index)), problems ) && valid;
            field_index++;
        }
    }
    return valid;
}

//TODO This currently does not invalidate unrecognized field types
bool Validator::typeCheck(const std::string& value, const std::string& type, std::vector<std::string>* problems) const {

    bool valid = true;

    if(type == "Integer"){
        try {
            boost::lexical_cast<int>( value.c_str());
        }
        catch(boost::bad_lexical_cast &) {
            valid = false;
            if (problems) {
                problems->push_back( "Could not cast: \'" + value + "\' as Integer" );
            }
        }
    } else if(type == "Float"){
        try {
            boost::lexical_cast<double>( value.c_str());
        }
        catch(boost::bad_lexical_cast &) {
            valid = false;
            if (problems) {
                problems->push_back( "Could not cast: \'" + value + "\' as Float" );
            }
        }
    } else if(type == "Character"){
        try {
            boost::lexical_cast<char>( value.c_str());
        }
        catch(boost::bad_lexical_cast &) {
            valid = false;
            if (problems) {
                problems->push_back( "Could not cast: \'" + value + "\' as Character" );
            }
        }
    } else if (type == "Flag"){
        //do nothing, Flag is allowed, but there is no data to cast
    } else if (type == "String"){
        //do nothing, the data is already in String format
    } else {
        valid = false;
        if (problems) {
            problems->push_back( "Invalid data type in header: \'" + type + "\'" );
        }
    }
    return valid;
}

VCF_NAMESPACE_END
