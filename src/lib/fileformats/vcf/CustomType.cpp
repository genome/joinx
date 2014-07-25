#include "CustomType.hpp"
#include "common/Tokenizer.hpp"

#include <boost/assign.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <set>
#include <stdexcept>

using boost::format;
using boost::lexical_cast;
using namespace std;

BEGIN_NAMESPACE(Vcf)

CustomType::CustomType()
    : _numberType(VARIABLE_SIZE)
    , _number(0)
    , _type(STRING)
{
}

CustomType::CustomType(const string& raw) {
    Tokenizer<string> t(raw, ",=\"");
    set<string> required = boost::assign::list_of("ID")("Number")("Type")("Description");

    string id;
    while (t.extract(id)) {
        // ="  or " at EOS will give us a null token, skip it
        if (id.empty())
            continue;

        string value;
        string tok;
        if (!t.extract(value))
            throw runtime_error(str(format("Failed to extract token while parsing custom type %1%") %raw));

        if (t.lastDelim() == '"') {
            do {
                string tmp;
                if (!t.extract(tmp))
                    break;
                value += tmp;
                if (t.lastDelim() != '"')
                    value += t.lastDelim();
            } while (t.lastDelim() != '"');
        }

        if (required.erase(id) != 1)
            throw runtime_error(str(format("Unexpected field id '%1%' while parsing custom time %2%") %id %raw));

        if (id == "ID") {
            _id = value;
        } else if (id == "Number") {
            _numberType = stringToNumber(value, _number);
        } else if (id == "Type" ) {
            _type = stringToType(value);
        } else if (id == "Description") {
            _description = value;
        }
    }
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

string CustomType::numberToString(NumberType t, uint32_t n) {
    switch (t) {
        case FIXED_SIZE:
            return lexical_cast<string>(n);
            break;
        case PER_ALLELE:     return "A"; break;
        case PER_ALLELE_REF: return "R"; break;
        case PER_GENOTYPE:   return "G"; break;
        case VARIABLE_SIZE:  return "."; break;
        default:
            throw runtime_error("Invalid custom VCF type!");
            break;
    }
}

string CustomType::typeToString(DataType t) {
    switch (t) {
        case INTEGER: return "Integer"  ; break;
        case FLOAT:   return "Float"    ; break;
        case CHAR:    return "Character"; break;
        case STRING:  return "String"   ; break;
        case FLAG:    return "Flag"     ; break;
        default:
            throw runtime_error("Invalid custom VCF type!");
            break;
    }
}


string CustomType::toString() const {
    stringstream ss;
    ss << "ID=" << _id
        << ",Number=" << numberToString(_numberType, _number)
        << ",Type=" << typeToString(_type)
        << ",Description=\"" << description() << "\"";
    return ss.str();
}

CustomType::NumberType CustomType::stringToNumber(const std::string& s, uint32_t& n) {
    n = 0;
    if (s == "A") {
        return PER_ALLELE;
    } else if (s == "R") {
        return PER_ALLELE_REF;
    } else if (s == "G") {
        return PER_GENOTYPE;
    } else if (s == ".") {
        return VARIABLE_SIZE;
    } else {
        if (!s.empty() && s[0] == '-')
            return VARIABLE_SIZE;

        n = lexical_cast<uint32_t>(s);
        return FIXED_SIZE;
    }
}

CustomType::DataType CustomType::stringToType(const std::string& s) {
    if (s == "Integer") {
        return INTEGER;
    } else if (s == "Float") {
        return FLOAT;
    } else if (s == "Character") {
        return CHAR;
    } else if (s == "String") {
        return STRING;
    } else if (s == "Flag") {
        return FLAG;
    } else {
        throw runtime_error(str(format("Unknown data type '%1%'. "
            "Expected one of: Integer, Float, Character, String, Flag") % s));
    }
}

void CustomType::emptyRepr(std::ostream& s) const {
    if (_type == FLAG)
        return;

    s << '.';
}

END_NAMESPACE(Vcf)
