#include "FastaIndexGenerator.hpp"

#include "common/compat.hpp"

#include <boost/format.hpp>

#include <algorithm>
#include <stdexcept>
#include <cctype>

using boost::format;


namespace {
    const char SEQ_BEGIN_CHAR = '>';
}


FastaIndexGenerator::FastaIndexGenerator(char const* data, size_t len)
    : _beg(data)
    , _pos(data)
    , _end(data+len)
{
}

void FastaIndexGenerator::extractName(Entry& e) {
    if (*_pos != SEQ_BEGIN_CHAR) {
        throw std::runtime_error(str(format(
            "Fasta sequence line begins with '%1%', expected '%2%"
            ) %*_pos %SEQ_BEGIN_CHAR));
    }
    ++_pos;

    char const* space = std::find_if(_pos, _end, ::isspace);
    e.name = std::string(_pos, space);
    // skip comment
    _pos = std::find(_pos, _end, '\n');
    _pos = std::find_if(_pos, _end, ::isgraph);
}

void FastaIndexGenerator::countLines(Entry& e) {
    if (*_pos == SEQ_BEGIN_CHAR) {
        throw std::runtime_error(str(format(
            "Empty sequence '%1%' in fasta file") %e.name));
    }

    char const* newline = std::find_if(_pos, _end, ::isspace);
    e.len = e.lineLength = e.lineBasesLength = newline - _pos;
    _pos = newline;
    while (::isspace(*_pos)) {
        ++e.lineLength;
        ++_pos;
    }

    while (*_pos != SEQ_BEGIN_CHAR) {
        char const* newline = std::find_if(_pos, _end, ::isspace);
        size_t len = newline - _pos;
        e.len += len;
        _pos = std::find_if(newline, _end, ::isgraph);
        if (len != e.lineBasesLength)
            break;
    }
    // we should be at the next seq now
    if (_pos != _end && *_pos != SEQ_BEGIN_CHAR) {
        throw std::runtime_error("Uneven line length");
    }
}

std::unique_ptr<FastaIndex> FastaIndexGenerator::generate() {
    auto index = std::make_unique<FastaIndex>();
    while (_pos < _end) {
        Entry e;
        extractName(e);
        e.offset = _pos - _beg;
        countLines(e);
        index->_sequenceOrder.push_back(e.name);
        index->_entries[e.name] = e;
    }
    return std::move(index);
}

