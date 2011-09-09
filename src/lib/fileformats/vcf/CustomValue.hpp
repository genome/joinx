#pragma once

#include "CustomType.hpp"
#include "common/Tokenizer.hpp"
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
    template<typename T>
    bool set(const std::string& value) {
        _type.typecheck<T>();
        SizeType idx = 0;
        Tokenizer<char> t(value, ',');
        std::string tok;
        T tmp;
        if (!t.extract(tmp))
            return false;

        do {
            _type.validateIndex(idx);
            ensureCapacity(idx+1);
            _values[idx++] = tmp;
        } while (t.extract(tmp));

        return t.eof();
    }

    void ensureCapacity(uint32_t size);

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
    ensureCapacity(idx+1);
    _values[idx] = value;
}

inline void CustomValue::ensureCapacity(uint32_t size) {
    if (size > _values.size())
        _values.resize(size);
}

VCF_NAMESPACE_END
