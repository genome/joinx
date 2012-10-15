#include "WiggleReader.hpp"
#include "common/Tokenizer.hpp"

#include <boost/format.hpp>
#include <cstring>
#include <stdexcept>
#include <vector>

using boost::format;
using namespace std;

namespace {
    bool startsWith(std::string const& s, char const* value) {
        size_t valLen = strlen(value);
        return s.size() >= valLen && s.compare(0, valLen, value) == 0;
    }
}

WiggleReader::WiggleReader(InputStream& in, bool stripChr)
    : _in(in)
    , _stripChr(stripChr)
    , _posBeg(0)
    , _pos(0)
    , _step(0)
    , _span(0)
    , _lineNum(0)
    , _ready(false)
{
}

bool WiggleReader::next(Bed& value) {
    while (!_in.eof()) {
        _in.getline(_line);

        ++_lineNum;
        bool ret = false;
        if (startsWith(_line, "track")) {
            newTrack();
            if (_ready) {
                getEntry(value);
                ret = true;
            }
        } else if (startsWith(_line, "variableStep")) {
            throw runtime_error(errorMessage("variableStep is unsupported"));
        } else if (startsWith(_line, "fixedStep")) {
            if (_ready) {
                ret = true;
                getEntry(value);
            }
            fixedStep();
        } else {
            if (!_ready)
                throw runtime_error(errorMessage("expected: fixedStep or variableStep"));

            if (!_last.empty() && (!(_last == _line) || _step != _span)) {
                getEntry(value);
                ret = true;
            }

            _last = _line;
            _pos += _span;
        }
        if (ret)
            return true;
    }

    return false;
}

void WiggleReader::getEntry(Bed& value) {
    vector<string> extra;
    extra.emplace_back(_last);
    value = Bed(_chrom, _posBeg-1, _pos - 1, extra);
    if (_step != _span) {
        _posBeg += _step;
        _pos = _posBeg;
    } else {
        _posBeg = _pos;
    }

}

std::string WiggleReader::errorMessage(std::string const& msg) const {
    return str(format("Error in %1% at line %2%: %3% -- %4%") %_in.name() %_lineNum %_line %msg);
}

void WiggleReader::fixedStep() {
    // set defaults
    _span = 1;
    _step = 1;

    _last.clear();

    Tokenizer<char> tok(_line, ' ');
    JxString token;
    tok.extract(token); // strip off leading fixedStep

    while (tok.extract(token)) {
        // The tokens are key=value pairs.
        JxString key;
        Tokenizer<char> kvtok(token, '=');
        if (!kvtok.extract(key))
            throw runtime_error(errorMessage("expected key=value pairs"));

        if (key == "chrom") {
            if (!kvtok.extract(_chrom))
                throw runtime_error(errorMessage("Invalid chrom"));

            if (_stripChr && _chrom.compare(0, 3, "chr") == 0)
                _chrom.erase(0, 3);

        } else if (key == "start") {
            if (!kvtok.extract(_posBeg))
                throw runtime_error(errorMessage("invalid pos"));
            _pos = _posBeg;
        } else if (key == "step") {
            if (!kvtok.extract(_step))
                throw runtime_error(errorMessage("invalid step"));
        } else if (key == "span") {
            if (!kvtok.extract(_span))
                throw runtime_error(errorMessage("invalid span"));
        }
    }
    _ready = true;
}

void WiggleReader::newTrack() {
    // we don't care about tracks
}

bool WiggleReader::eof() const {
    return _in.eof();
}

