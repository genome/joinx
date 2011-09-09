#include "CustomValue.hpp"

#include <boost/format.hpp>

using boost::format;
using namespace std;

VCF_NAMESPACE_BEGIN

CustomValue::CustomValue(const CustomType& type)
    : _type(type)
{
}

CustomValue::CustomValue(const CustomType& type, const string& value)
    : _type(type)
{
    if (value == ".")
        return;

    bool rv = false;
    switch (_type.type()) {
        case CustomType::INTEGER:
            rv = set<int64_t>(value);
            break;

        case CustomType::FLOAT:
            rv = set<double>(value);
            break;

        case CustomType::CHAR:
            rv = set<char>(value);
            break;

        case CustomType::STRING:
            rv = set<string>(value);
            break;

        case CustomType::FLAG:
            // no need for an extra bool
            // the values very presence is an indication of its truthiness
            break;

        default:
            throw runtime_error("Invalid custom VCF type!");
            break;
    }

    if (rv == false)
        throw runtime_error(str(format("Failed to coerce value %1% into %2%")
            %value %CustomType::typeToString(_type.type())));
}

const CustomValue::ValueType* CustomValue::getAny(SizeType idx) const {
    _type.validateIndex(idx);
    if (idx >= _values.size())
        return 0;
    return &_values[idx];
}

const CustomType& CustomValue::type() const {
    return _type;
}

CustomValue::SizeType CustomValue::size() const {
    return _values.size();
}

bool CustomValue::empty() const {
    return _values.empty();
}

VCF_NAMESPACE_END
