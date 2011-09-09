#pragma once

#include "namespace.hpp"

#include <boost/any.hpp>
#include <cstdint>
#include <string>
#include <vector>

VCF_NAMESPACE_BEGIN

class CustomType {
public:
    enum NumberType {
        FIXED_SIZE,
        PER_ALLELE,
        PER_GENOME,
        VARIABLE_SIZE
    };

    enum DataType {
        INTEGER,
        FLOAT,
        CHAR,
        STRING
    };

    CustomType();
    explicit CustomType(const std::string& raw);
    CustomType(
        const std::string& id,
        NumberType numberType,
        uint32_t number,
        DataType type,
        const std::string& description
        );

    NumberType numberType() const;
    uint32_t number() const;
    DataType type() const;
    const std::string& description() const;

    bool operator==(const CustomType& rhs) const {
        return 
            _id == rhs._id &&
            _numberType == rhs._numberType &&
            _number == rhs._number &&
            _type == rhs._type &&
            _description == rhs._description;
    }

    bool operator!=(const CustomType& rhs) const {
        return !(*this == rhs);
    }


    void validateIndex(uint32_t idx) const;

    std::string numberString() const;
    std::string typeString() const;
    std::string toString() const;

protected:
    std::string _id;
    NumberType _numberType;
    uint32_t _number;
    DataType _type;
    std::string _description;
};

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


inline CustomType::NumberType CustomType::numberType() const {
    return _numberType;
}

inline uint32_t CustomType::number() const {
    return _number;
}

inline CustomType::DataType CustomType::type() const {
    return _type;
}

inline const std::string& CustomType::description() const {
    return _description;
}

inline const CustomValue::ValueType* CustomValue::getAny(SizeType idx) const {
    _type.validateIndex(idx);
    if (idx >= _values.size())
        return 0; 
    return &_values[idx];
}

template<typename T>
inline const T* CustomValue::get(SizeType idx) const {
    _type.validateIndex(idx);
    if (idx >= _values.size())
        return 0;
    return boost::any_cast<T>(&_values[idx]);
}

template<typename T>
inline void CustomValue::set(SizeType idx, const T& value) {
    _type.validateIndex(idx);
    if (idx >= _values.size())
        _values.resize(idx+1);
    _values[idx] = value;
}

inline const CustomType& CustomValue::type() const {
    return _type;
}

inline CustomValue::SizeType CustomValue::size() const {
    return _values.size();
}

inline bool CustomValue::empty() const {
    return _values.empty();
}

VCF_NAMESPACE_END
