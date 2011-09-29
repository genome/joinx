#pragma once

#include "MergeActions.hpp"
#include "namespace.hpp"

#include <map>
#include <string>

VCF_NAMESPACE_BEGIN

class CustomType;
class CustomValue;
class Entry;
class Header;

class MergeStrategy {
public:
    explicit MergeStrategy(const Header* header);
    MergeStrategy(const Header* header, std::istream& description);
    virtual ~MergeStrategy();

    void setHeader(const Header* header);
    CustomValue mergeInfo(const std::string& which, const Entry* begin, const Entry* end) const;

    void setAction(const std::string& id, MergeActions::Base* action);

protected:
    const Header* _header;
    std::map<std::string, MergeActions::Base*> _info;
};

VCF_NAMESPACE_END
