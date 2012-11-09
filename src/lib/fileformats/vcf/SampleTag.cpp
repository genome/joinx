#include "SampleTag.hpp"

#include <boost/format.hpp>
#include <boost/spirit/home/phoenix/container.hpp>
#include <boost/fusion/include/std_pair.hpp>
#include <boost/spirit/include/karma.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/spirit/include/qi.hpp>
#include <iostream>

namespace phx = boost::phoenix;
namespace spirit = boost::spirit;
namespace karma = boost::spirit::karma;
namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;

using namespace std;
using boost::format;

namespace {
    template<typename Iterator>
    struct SampleTagGrammar
        : public qi::grammar<Iterator, map<string, string>()>
    {
        SampleTagGrammar(std::vector<std::string>& fieldOrder)
            : SampleTagGrammar::base_type(start)
        {
            // a key is one or more characters excluding a few that we don't
            // really like (<, >, i am looking at you).
            key = +(qi::char_ - qi::char_(" ,<>="));

            // a quoted string is a double quote followed by zero or more
            // non-quote characters followed by a closing double quote.
            quotedString = 
                qi::char_('"')
                > *(~qi::char_('"'))
                > qi::char_('"')
            ;

            // a value string is a quoted string, or a sequence of one or more
            // any 'allowable' characters (double quotes, commas, spaces, <, or
            // > need not apply).
            valueString = 
                quotedString
                | +(qi::char_ - qi::char_("\", <>"))
                ;

            // a value array is a comma separated list of zero or more value
            // strings (this admits the possibility of the empty array <>).
            // individual values in the array may not be empty, however, so
            // <x,,y> is illegal (the thinking is that <x,.,y> should be used
            // instead). placing a - before valueString below removes this
            // condition.
            valueArray = 
                qi::char_('<')
                > -(*valueString > *(qi::char_(',') > *valueString))
                > qi::char_('>')
            ;

            // a value is either a value string or value array.
            value = valueString | valueArray;

            // a "pear" is a key optionally followed by =value. this admits
            // the possibility of flags, e.g., 'id=a,foo,bar=x' where foo is
            // a flag (key with no value).
            pear %=
                // we attach an additional semantic action when keys are
                // processed: we push them onto the fieldOrder vector so
                // order can be preserved if we decide to print this thing
                // out later.
                key[ phx::push_back(phx::ref(fieldOrder), qi::_1) ]
                > -('=' > value);

            start %= pear % ',';

            qi::on_error<qi::fail> (
                start,
                std::cerr
                    << phx::val("Parse error: expected ")
                    << qi::_4
                    << phx::val(" here: '")
                    << phx::construct<std::string>(qi::_3, qi::_2)
                    << phx::val("'\n")
            );
        }

        qi::rule<Iterator, std::map<std::string, std::string>()> start;
        qi::rule<Iterator, std::pair<std::string, std::string>()> pear;
        qi::rule<Iterator, std::string()> key;
        qi::rule<Iterator, std::string()> valueString;
        qi::rule<Iterator, std::string()> quotedString;
        qi::rule<Iterator, std::string()> valueArray;
        qi::rule<Iterator, std::string()> value;
    };
}

BEGIN_NAMESPACE(Vcf)

SampleTag::SampleTag() {
}

SampleTag::SampleTag(std::string const& raw) {
    typedef string::const_iterator It;
    It iter = raw.begin();
    It end = raw.end();
    SampleTagGrammar<It> grammar(_fieldOrder);
    bool rv = qi::parse(iter, end, grammar, _fields);
    bool done = iter == end;
    if (!rv || !done) {
        throw runtime_error(str(format(
            "Failed to parse sample tag in vcf header: '%1%'\n"
            "Got as far as: %2%"
            ) %raw %toString()));
    }
}

void SampleTag::toStream(std::ostream& s) const {
    s << "##SAMPLE=<";
    for (auto i = _fieldOrder.begin(); i != _fieldOrder.end(); ++i) {
        if (i != _fieldOrder.begin())
            s << ',';
        s << *i;
        auto iter = _fields.find(*i);
        if (iter == _fields.end())
            continue;
        s << "=" << iter->second;
    }
    s << ">";
}

std::string SampleTag::toString() const {
    stringstream ss;
    ss << *this;
    return ss.str();
}

void SampleTag::set(std::string const& name, std::string const& value) {
    auto inserted = _fields.insert(make_pair(name, value));
    if (inserted.second)
        _fieldOrder.push_back(name);
}

std::string const& SampleTag::id() const {
    auto iter = _fields.find("ID");
    if (iter == _fields.end())
        throw runtime_error("Sample tag with no ID!");
    return iter->second;
}

END_NAMESPACE(Vcf)

std::ostream& operator<<(std::ostream& s, Vcf::SampleTag const& sampleTag) {
    sampleTag.toStream(s);
    return s;
}

