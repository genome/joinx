#include "CustomValue.hpp"

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

#include <utility>

using boost::format;
using boost::lexical_cast;
using namespace std;

BEGIN_NAMESPACE(Vcf)

CustomValue::CustomValue()
    : _type(0)
{
}

CustomValue::CustomValue(CustomValue const& other)
    : _type(other._type)
    , _values(other._values)
{
}

CustomValue::CustomValue(CustomValue&& other)
    : _type(other._type)
    , _values(std::move(other._values))
{
}

CustomValue::CustomValue(const CustomType* type, const std::vector<ValueType>&& values)
    : _type(type)
    , _values(std::move(values))
{
}

CustomValue& CustomValue::operator=(CustomValue const& other) {
    _type = other._type;
    _values = other._values;
    return *this;
}

CustomValue& CustomValue::operator=(CustomValue&& other) {
    _type = other._type;
    _values.swap(other._values);
    return *this;
}

CustomValue::CustomValue(const CustomType* type)
    : _type(type)
{
}

CustomValue::CustomValue(const CustomType* type, const string& value)
    : _type(type)
{
    if (value == ".")
        return;

    bool rv = false;
    switch (type->type()) {
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
            rv = true;
            break;

        default:
            throw runtime_error("Invalid custom VCF type!");
            break;
    }

    if (rv == false)
        throw runtime_error(str(format("Failed to coerce value '%1%' into %2% for field '%3%'")
            %value %CustomType::typeToString(type->type()) %type->id()));
}

const CustomValue::ValueType* CustomValue::getAny(SizeType idx) const {
    type().validateIndex(idx);
    if (idx >= _values.size() || _values[idx].which() == 0) {
        return 0;
    }
    return &_values[idx];
}

const CustomType& CustomValue::type() const {
    if (!_type)
        throw runtime_error("Attempted to use Vcf CustomValue with uninitialized type");
    return *_type;
}

CustomValue::SizeType CustomValue::size() const {
    return _values.size();
}

bool CustomValue::empty() const {
    return _values.empty();
}

// This gets called to notify existing values how many alleles there are.
// This gives them the opportunity to throw an exception if there are more
// values than there are supposed to be
void CustomValue::setNumAlts(uint32_t n) {
    uint32_t maxValue = std::numeric_limits<uint32_t>::max();
    if (type().numberType() == CustomType::PER_ALLELE) {
        maxValue = n;
    }
    else if (type().numberType() == CustomType::PER_ALLELE_REF) {
        maxValue = n + 1;
    }

    if (_values.size() > maxValue) {
        std::stringstream ss;
        ss << (*this);
        throw std::runtime_error(str(format(
            "Too many values in per-alt value '%1%' for field '%2%', "
            "expected at most %3%"
            ) % ss.str() % type().id() % n));
    }

    if (type().numberType() == CustomType::PER_ALLELE) {
        _values.resize(n);
    }
    else if (type().numberType() == CustomType::PER_ALLELE_REF) {
        _values.resize(n + 1);
    }
}

std::string CustomValue::getString(SizeType idx) const {
    type().validateIndex(idx);
    if (idx >= _values.size() || _values[idx].empty())
        return ".";

    stringstream ss;
    switch (type().type()) {
        case CustomType::INTEGER:
             ss << *get<int64_t>(idx);
            break;

        case CustomType::FLOAT:
            ss << *get<double>(idx);
            break;

        case CustomType::CHAR:
            ss << *get<char>(idx);
            break;

        case CustomType::STRING:
            return *get<string>(idx);
            break;

        case CustomType::FLAG:
            // no need for an extra bool
            // the values very presence is an indication of its truthiness
            break;

        default:
            throw runtime_error("Invalid custom VCF type!");
            break;
    }

    return ss.str();
}

void CustomValue::toStream(ostream& s) const {
    if (empty()) {
        s << ".";
        return;
    }

    switch (type().type()) {
        case CustomType::INTEGER:
            toStream_impl<int64_t>(s);
            break;

        case CustomType::FLOAT:
            toStream_impl<double>(s);
            break;

        case CustomType::CHAR:
            toStream_impl<char>(s);
            break;

        case CustomType::STRING:
            toStream_impl<string>(s);
            break;

        case CustomType::FLAG:
            toStream_impl<bool>(s);
            break;

        default:
            throw runtime_error("Invalid custom VCF type!");
            break;
    }
}

std::string CustomValue::toString() const {
    stringstream ss;
    toStream(ss);
    return ss.str();
}

void CustomValue::append(const CustomValue& other) {
    if (other.type() != type())
        throw runtime_error(str(format("Attempted to concatenate conflicting custom types: %1% and %2%")
            %type().toString() %other.type().toString()));
    SizeType idx = size();
    SizeType newSize = size() + other.size();
    type().validateIndex(newSize-1);
    ensureCapacity(newSize);
    for (SizeType i = 0; i < other.size(); ++i)
        _values[idx++] = other._values[i];
}

CustomValue& CustomValue::operator+=(const CustomValue& rhs) {
    if (!type().numeric()) {
        throw runtime_error(str(format(
            "Attempted to perform addition on non-numeric custom value type %1%"
            ) %type().id()));
    }

    if (type().numberType() == CustomType::VARIABLE_SIZE) {
        throw runtime_error(str(format("Attempted to perform summation on variable length field %1%")
            %type().id()));
    }

    if (type() != rhs.type()) {
        throw runtime_error(str(format("Attempted to add dissimilar types: %1% and %2%")
            %type().id() %rhs.type().id()));
    }

    if (type().number() != rhs.type().number()) {
        throw runtime_error(str(format(
            "Attempted to perform summation on %1% elements of differeng size: %2% vs %3%"
            ) %type().id() %type().number() %rhs.type().number()));
    }

    if (type().type() == CustomType::INTEGER) {
        add<int64_t>(rhs);
    } else if (type().type() == CustomType::FLOAT) {
        add<double>(rhs);
    } else {
        throw runtime_error(str(format("Unexpected type for CustomValue addition: %1%")
            %CustomType::typeToString(type().type())));
    }

    return *this;
}

END_NAMESPACE(Vcf)

std::ostream& operator<<(std::ostream& s, const Vcf::CustomValue& v) {
    v.toStream(s);
    return s;
}
