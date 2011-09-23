#include "MergeStrategy.hpp"
#include "Entry.hpp"
#include "Header.hpp"
#include "CustomValue.hpp"

#include <boost/format.hpp>
#include <functional>
#include <set>
#include <stdexcept>

using boost::format;
using namespace std;
using namespace std::placeholders;

VCF_NAMESPACE_BEGIN

typedef MergeActions::Base::FetchFunc FetchFunc;

MergeStrategy::MergeStrategy(const Header* header)
    : _header(header)
{
}

MergeStrategy::~MergeStrategy() {
    for (auto i = _info.begin(); i != _info.end(); ++i)
        delete i->second;
}

void MergeStrategy::setHeader(const Header* header) {
    _header = header;
}

CustomValue MergeStrategy::mergeInfo(const string& which, const Entry* begin, const Entry* end) const {
    const CustomValue* (Entry::*fetchInfo)(const string&) const = &Entry::info;
    FetchFunc fetch = bind(fetchInfo, _1, which);
    const CustomType* type = _header->infoType(which);
    if (!type)
        throw runtime_error(str(format("Unknown datatype for info field '%1%'") %which));

    auto iter = _info.find(which);
    if (iter != _info.end())
        return (*iter->second)(type, fetch, begin, end);

    if (type->numberType() == CustomType::VARIABLE_SIZE)
        return MergeActions::Concatenate()(type, fetch, begin, end);
    else
        return MergeActions::Equality()(type, fetch, begin, end);

//    throw runtime_error(str(format("Unknown merge strategy for info field '%1%'") %which));
}

VCF_NAMESPACE_END
