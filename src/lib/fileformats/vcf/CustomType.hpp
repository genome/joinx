#pragma once

#include "common/namespaces.hpp"
#include "common/cstdint.hpp"

#include <boost/format.hpp>
#include <string>

BEGIN_NAMESPACE(Vcf)

class CustomType {
public:
    enum NumberType {
        FIXED_SIZE,
        PER_ALLELE,
        PER_GENOTYPE,
        VARIABLE_SIZE
    };

    enum DataType {
        INTEGER,
        FLOAT,
        CHAR,
        STRING,
        FLAG
    };

    static std::string numberToString(NumberType t, uint32_t n);
    static std::string typeToString(DataType t);
    static NumberType stringToNumber(const std::string& s, uint32_t& n);
    static DataType stringToType(const std::string& s);

    CustomType();
    explicit CustomType(const std::string& raw);
    CustomType(
        const std::string& id,
        NumberType numberType,
        uint32_t number,
        DataType type,
        const std::string& description
        );

    const std::string& id() const;
    NumberType numberType() const;
    uint32_t number() const;
    DataType type() const;
    const std::string& description() const;

    bool isScalar() const {
        return _numberType == FIXED_SIZE && _number == 1;
    }

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

    bool numeric() const {
        return _type == INTEGER || _type == FLOAT;
    }

    template<typename T>
    void typecheck() const;

    void validateIndex(uint32_t idx) const;

    std::string toString() const;

    void emptyRepr(std::ostream& s) const;

protected:
    std::string _id;
    NumberType _numberType;
    uint32_t _number;
    DataType _type;
    std::string _description;
};

inline const std::string& CustomType::id() const {
    return _id;
}

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

template<>
inline void CustomType::typecheck<double>() const {
    using boost::format;
    if (_type != FLOAT)
        throw std::runtime_error(str(format("VCF Custom type error, tried to convert %1% to Float") %typeToString(_type)));
}

template<>
inline void CustomType::typecheck<std::string>() const {
    using boost::format;
    if (_type != STRING)
        throw std::runtime_error(str(format("VCF Custom type error, tried to convert %1% to String") %typeToString(_type)));
}

template<>
inline void CustomType::typecheck<char>() const {
    using boost::format;
    if (_type != CHAR)
        throw std::runtime_error(str(format("VCF Custom type error, tried to convert %1% to Character") %typeToString(_type)));
}

template<>
inline void CustomType::typecheck<bool>() const {
    using boost::format;
    if (_type != FLAG)
        throw std::runtime_error(str(format("VCF Custom type error, tried to convert %1% to Flag") %typeToString(_type)));
}

template<>
inline void CustomType::typecheck<int64_t>() const {
    using boost::format;
    if (_type != INTEGER)
        throw std::runtime_error(str(format("VCF Custom type error, tried to convert %1% to Integer") %typeToString(_type)));
}

template<>
inline void CustomType::typecheck<uint64_t>() const {
    typecheck<int64_t>();
}

template<>
inline void CustomType::typecheck<int32_t>() const {
    typecheck<int64_t>();
}

template<>
inline void CustomType::typecheck<uint32_t>() const {
    typecheck<int64_t>();
}

END_NAMESPACE(Vcf)
