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
    virtual ~MergeStrategy();

    void setHeader(const Header* header);
    CustomValue mergeInfo(const std::string& which, const Entry* begin, const Entry* end) const;

protected:
    const Header* _header;
    std::map<std::string, MergeActions::Base*> _info;
};

VCF_NAMESPACE_END
