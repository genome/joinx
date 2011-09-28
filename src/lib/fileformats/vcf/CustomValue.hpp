#pragma once

#include "CustomType.hpp"
#include "common/Tokenizer.hpp"
#include "namespace.hpp"

#include <boost/any.hpp>
#include <boost/lexical_cast.hpp>
#include <string>
#include <vector>
#include <ostream>

VCF_NAMESPACE_BEGIN

class CustomValue {
public:
    typedef boost::any ValueType;
    typedef std::vector<ValueType>::size_type SizeType;

    CustomValue();
    explicit CustomValue(const CustomType* type);
    CustomValue(const CustomType* type, const std::string& value);

    const CustomType& type() const;
    SizeType size() const;
    bool empty() const;
    const ValueType* getAny(SizeType idx) const;

    template<typename T>
    const T* get(SizeType idx) const;

    template<typename T>
    void set(SizeType idx, const T& value);

    void setString(SizeType idx, const std::string& value);
    std::string getString(SizeType idx) const;
    void toStream(std::ostream& s) const;
    std::string toString() const;
    std::string toString(SizeType idx) const;

    void append(const CustomValue& other);

    bool operator==(const CustomValue& rhs) const;
    bool operator!=(const CustomValue& rhs) const;

protected:
    template<typename T>
    bool set(const std::string& value) {
        type().typecheck<T>();
        SizeType idx = 0;
        Tokenizer<char> t(value, ',');
        std::string tok;
        if (!t.extract(tok))
            return false;

        do {
            type().validateIndex(idx);
            ensureCapacity(idx+1);
            if (tok == ".") {
                _values[idx++] = boost::any();
            } else {
                try {
                    T tmp = boost::lexical_cast<T>(tok);
                    _values[idx++] = tmp;
                } catch (const boost::bad_lexical_cast& e) {
                    return false;
                }
            }
        } while (t.extract(tok));

        return t.eof();
    }

    void ensureCapacity(uint32_t size);

protected:
    const CustomType* _type;
    std::vector<ValueType> _values;
};

template<typename T>
inline const T* CustomValue::get(SizeType idx) const {
    type().typecheck<T>();
    type().validateIndex(idx);
    if (idx >= _values.size())
        return 0;
    return boost::any_cast<T>(&_values[idx]);
}

template<typename T>
inline void CustomValue::set(SizeType idx, const T& value) {
    type().typecheck<T>();
    type().validateIndex(idx);
    ensureCapacity(idx+1);
    _values[idx] = value;
}

inline void CustomValue::ensureCapacity(uint32_t size) {
    if (size > _values.size())
        _values.resize(size);
}

inline bool CustomValue::operator==(const CustomValue& rhs) const {
    // TODO: make this more efficient instead of using toString
    // the problem is you can't compare boost::any objects without casting to
    // the proper type, which we'd have to switch for
    return *_type == rhs.type() &&
        toString() == rhs.toString();
}

inline bool CustomValue::operator!=(const CustomValue& rhs) const {
    return !(*this == rhs);
}

VCF_NAMESPACE_END

std::ostream& operator<<(std::ostream& s, const Vcf::CustomValue& v);
