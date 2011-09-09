#include "CustomType.hpp"

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <stdexcept>

using boost::format;
using boost::lexical_cast;
using namespace std;

VCF_NAMESPACE_BEGIN

CustomType::CustomType()
    : _numberType(VARIABLE_SIZE)
    , _number(0)
    , _type(STRING)
{
}

CustomType::CustomType(const string& raw) {
}

CustomType::CustomType(
        const string& id,
        NumberType numberType,
        uint32_t number,
        DataType type,
        const string& description
        )
    : _id(id)
    , _numberType(numberType)
    , _number(number)
    , _type(type)
    , _description(description)
{
}

void CustomType::validateIndex(uint32_t idx) const {
    using boost::format;
    if (_numberType == FIXED_SIZE && idx >= _number)
        throw runtime_error(
            str(format("Request for index %1% from type %2% is invalid") %idx %toString()
            ));
    
}

string CustomType::numberString() const {
    switch (_numberType) {
        case FIXED_SIZE:
            return lexical_cast<string>(_number);
            break;
        case PER_ALLELE:    return "A"; break;
        case PER_GENOME:    return "G"; break;
        case VARIABLE_SIZE: return "."; break;
        default:
            throw runtime_error("Invalid custom VCF type!");
            break;
    }
}

string CustomType::typeString() const {
    switch (_type) {
        case INTEGER: return "Integer"  ; break;
        case FLOAT:   return "Float"    ; break;
        case CHAR:    return "Character"; break;
        case STRING:  return "String"   ; break;
        default:
            throw runtime_error("Invalid custom VCF type!");
            break;
    }
}


string CustomType::toString() const {
    stringstream ss;
    ss << "ID=" << _id 
        << ",Number=" << numberString() 
        << ",Type=" << typeString() 
        << ",Description=" << description();
    return ss.str();
}


CustomValue::CustomValue(const CustomType& type)
    : _type(type)
{
}

CustomValue::CustomValue(const CustomType& type, const string& value)
    : _type(type)
{
}

VCF_NAMESPACE_END
