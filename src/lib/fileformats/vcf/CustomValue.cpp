#include "CustomValue.hpp"

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

using boost::format;
using boost::lexical_cast;
using namespace std;

BEGIN_NAMESPACE(Vcf)

CustomValue::CustomValue()
    : _type(0)
{
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
    if (idx >= _values.size())
        return 0;
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

void CustomValue::setString(SizeType idx, const std::string& value) {
    type().validateIndex(idx);
    stringstream ss;
    if (value.empty() || value == ".") {
        _values.clear();
        return;
    }

    switch (type().type()) {
        case CustomType::INTEGER:
            set<int64_t>(value);
            break;

        case CustomType::FLOAT:
            set<double>(value);
            ss << *get<double>(idx);
            break;

        case CustomType::CHAR:
            set<char>(value);
            break;

        case CustomType::STRING:
            set<string>(value);
            break;

        case CustomType::FLAG:
            // no need for an extra bool
            // the values very presence is an indication of its truthiness
            break;

        default:
            throw runtime_error("Invalid custom VCF type!");
            break;
    }
}

void CustomValue::setNumAlts(uint32_t n) {
    if (type().numberType() == CustomType::PER_ALLELE)
        _values.resize(n);
}

std::string CustomValue::getString(SizeType idx) const {
    type().validateIndex(idx);
    stringstream ss;
    if (_values[idx].empty())
        return ".";

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
