#include "TranscriptStructure.hpp"

#include <boost/tokenizer.hpp>
#include <cassert>

using boost::lexical_cast;
using namespace std;

void TranscriptStructure::parseLine(const string& line, TranscriptStructure& ts) {
    typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
    boost::char_separator<char> sep(",", "", boost::keep_empty_tokens);
    tokenizer tokens(line, sep);
    unsigned idx = 0;
    for (tokenizer::iterator iter = tokens.begin(); iter != tokens.end(); ++iter, ++idx) {
        assert(idx < NUM_FIELDS);
        ts._fields[idx] = *iter;
    }

    boost::char_separator<char> sepTab("\t", "", boost::keep_empty_tokens);
    tokenizer idTokens(ts.get(transcript_id), sepTab);
    tokenizer::iterator iter = idTokens.begin();
    ts._start = lexical_cast<uint64_t>(ts.get(structure_start));
    ts._end = lexical_cast<uint64_t>(ts.get(structure_stop));

    ts._line = line;
}

TranscriptStructure::TranscriptStructure() : _start(0), _end(0) {}

const string& TranscriptStructure::get(Field f) const {
    assert(f < NUM_FIELDS);
    return _fields[f];
}
