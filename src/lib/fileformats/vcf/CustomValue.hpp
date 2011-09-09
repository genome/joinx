#pragma once

#include "CustomType.hpp"
#include "namespace.hpp"

#include <boost/any.hpp>
#include <string>
#include <vector>

VCF_NAMESPACE_BEGIN

class CustomValue {
public:
    typedef boost::any ValueType;
    typedef std::vector<ValueType>::size_type SizeType;

    CustomValue();
    explicit CustomValue(const CustomType& type);
    CustomValue(const CustomType& type, const std::string& value);

    const CustomType& type() const;
    SizeType size() const;
    bool empty() const;
    const ValueType* getAny(SizeType idx) const;

    template<typename T>
    const T* get(SizeType idx) const;

    template<typename T>
    void set(SizeType idx, const T& value);

protected:
    const CustomType& _type;
    std::vector<ValueType> _values;
};

template<typename T>
inline const T* CustomValue::get(SizeType idx) const {
    _type.typecheck<T>();
    _type.validateIndex(idx);
    if (idx >= _values.size())
        return 0;
    return boost::any_cast<T>(&_values[idx]);
}

template<typename T>
inline void CustomValue::set(SizeType idx, const T& value) {
    _type.typecheck<T>();
    _type.validateIndex(idx);
    if (idx >= _values.size())
        _values.resize(idx+1);
    _values[idx] = value;
}

VCF_NAMESPACE_END
