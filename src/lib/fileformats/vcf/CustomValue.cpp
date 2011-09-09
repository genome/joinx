#include "CustomValue.hpp"

using namespace std;

VCF_NAMESPACE_BEGIN

CustomValue::CustomValue(const CustomType& type)
    : _type(type)
{
}

CustomValue::CustomValue(const CustomType& type, const string& value)
    : _type(type)
{
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
