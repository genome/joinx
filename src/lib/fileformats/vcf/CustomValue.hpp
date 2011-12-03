#pragma once

#include "CustomType.hpp"
#include "common/MultiType.hpp"
#include "common/Tokenizer.hpp"
#include "common/namespaces.hpp"

#include <algorithm>
#include <functional>
#include <string>
#include <vector>
#include <ostream>

BEGIN_NAMESPACE(Vcf)

class CustomValue {
public:
    typedef MultiType ValueType;
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

    CustomValue& operator+=(const CustomValue& rhs);
    bool operator==(const CustomValue& rhs) const;
    bool operator!=(const CustomValue& rhs) const;

protected:
    template<typename T>
    void add(const CustomValue& rhs) {
        ensureCapacity(rhs.size());
        for (SizeType i = 0; i < type().number(); ++i) {
            T val(0);
            const T* a = get<T>(i);
            const T* b = rhs.get<T>(i);
            if (a != NULL) val += *a;
            if (b != NULL) val += *b;
            _values[i] = val;
        }
    }

    template<typename T>
    bool set(const std::string& value) {
        if (value.empty())
            return false;

        type().typecheck<T>();
        uint32_t nItems = std::count_if(value.begin(), value.end(),
            std::bind1st(std::equal_to<char>(), ','));
        _values.resize(nItems+1);

        Tokenizer<char> t(value, ',');

        const static std::string nullString(".");
        uint32_t idx(0);
        while (!t.eof()) {
            if (t.nextTokenMatches(""))
                return false;

            if (t.nextTokenMatches(nullString)) {
                t.advance();
            } else if (!t.extract(_values[idx].ref<T>()))
                return false;
            ++idx;
        }
        type().validateIndex(_values.size()-1);

        return t.eof();
    }


    void ensureCapacity(uint32_t size);

protected:
    template<typename T>
    void toStream_impl(std::ostream& s) const;

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
    return _values[idx].get<T>();
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

template<typename T>
inline void CustomValue::toStream_impl(ostream& s) const {
    if (empty()) {
        type().emptyRepr(s);
    }
    for (SizeType i = 0; i < size(); ++i) {
        if (i > 0)
            s << ",";
        T const* value(0);
        if (!_values[i].empty() && (value = _values[i].get<T>()))
            s << *value;
        else
            s << '.';
    }
}

END_NAMESPACE(Vcf)

std::ostream& operator<<(std::ostream& s, const Vcf::CustomValue& v);
