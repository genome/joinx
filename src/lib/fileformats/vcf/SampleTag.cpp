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
using namespace std;
using boost::format;

namespace {
    template<typename Iterator>
    struct SampleGrammar
        : public qi::grammar<Iterator, map<string, string>()>
    {
        SampleGrammar(std::vector<std::string>& fieldOrder)
            : SampleGrammar::base_type(start)
        {
            key = +(qi::char_ - qi::char_(" ,<>="));

            valueString = 
                (qi::char_('"') > +(qi::char_ - '"') > qi::char_('"'))
                | +(qi::char_ - qi::char_("\", <>"))
                ;

            value =
                valueString 
                | (qi::char_('<')
                    > -(valueString > *(qi::char_(',') > *valueString))
                    > qi::char_('>'))
                ;

            pear %=
                key[ phx::push_back(phx::ref(fieldOrder), qi::_1) ]
                > -('=' > value);

            start %= pear % ',';
        }

        qi::rule<Iterator, std::map<std::string, std::string>()> start;
        qi::rule<Iterator, std::pair<std::string, std::string>()> pear;
        qi::rule<Iterator, std::string()> key;
        qi::rule<Iterator, std::string()> valueString;
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
    SampleGrammar<It> grammar(_fieldOrder);
    bool rv = qi::parse(iter, end, grammar, _fields);
    bool done = iter == end;
    if (!rv || !done) {
        throw runtime_error(str(format(
            "Failed to parse sample tag in vcf header: '%1%"
            ) %raw));
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
