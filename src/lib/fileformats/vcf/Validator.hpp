#pragma once

#include "Header.hpp"
#include "namespace.hpp"

#include <string>
#include <vector>

VCF_NAMESPACE_BEGIN

class Entry;

// note: the header is held by reference, so it must not go out of scope before
// an instance of Validator holding it
class Validator {
public:
    Validator(const Header& header);

    // overloaded call operator so we can do: Validator v(hdr); v(myEntry);
    // if passed, the problems vector will describe the issues (if any)
    bool operator()(const Entry& e, std::vector<std::string>* problems = NULL) const;

protected:
    const Header& _header;

    // by default, the header entries are not necessarily sorted
    // we will optimize for the info/filter/format tags by copying and sorting
    // them to enable binary search.
    // note: Header::Category = vector<Map> and might
    Header::Category _info;
    Header::Category _filters;
    Header::Category _format;
    std::set<std::string> _info_list;
    std::set<std::string> _filters_list;
    std::set<std::string> _format_list;
};

VCF_NAMESPACE_END
