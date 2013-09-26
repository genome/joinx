#include "Fasta.hpp"
#include "InputStream.hpp"
#include "common/Exceptions.hpp"
#include "common/Tokenizer.hpp"
#include "common/UnknownSequenceError.hpp"

#include <boost/bind.hpp>
#include <boost/format.hpp>
#include <boost/function.hpp>

#include <algorithm>
#include <cctype>
#include <fstream>
#include <locale>
#include <stdexcept>
#include <vector>
#include <map>

using boost::format;
using namespace std;

namespace {
    const char* INDEX_EXTENSION = ".fai";
    const char SEQ_BEGIN_CHAR = '>';
}

class Fasta::Index {
public: // types
    struct Entry {
        Entry() : len(0), offset(0), lineBasesLength(0), lineLength(0) {}

        explicit Entry(std::string const& data) {
            Tokenizer<char> tok(data, '\t');
            if (!tok.extract(name)
                || !tok.extract(len)
                || !tok.extract(offset)
                || !tok.extract(lineBasesLength)
                || !tok.extract(lineLength))
            {
                throw runtime_error(str(format(
                    "Failed to parse fasta index line '%1%'") %data));
            }
        }

        string name;
        size_t len;
        size_t offset;
        size_t lineBasesLength;
        size_t lineLength;
    };

public: // functions

    Index() {
    }

    explicit Index(istream& in) {
        string line;
        while (getline(in, line)) {
            Entry e(line);
            _entryNames.push_back(e.name);
            _entries[e.name] = e;
        }
    }

    vector<string> const& names() const { return _entryNames; }
    Entry const* entry(std::string const& name) const {
        auto iter = _entries.find(name);
        if (iter != _entries.end())
            return &iter->second;
        return 0;
    }

    void save(std::ostream& out) {
        for (auto i = _entries.begin(); i != _entries.end(); ++i) {
            out << i->second.name << "\t"
                << i->second.len << "\t"
                << i->second.offset << "\t"
                << i->second.lineBasesLength << "\t"
                << i->second.lineLength << "\n";
        }
    }


protected: // data
    vector<string> _entryNames;
    map<string, Entry> _entries;
    friend class IndexGenerator;
};

class IndexGenerator {
public:
    typedef Fasta::Index::Entry Entry;

    IndexGenerator(char const* data, size_t len)
        : _beg(data)
        , _pos(data)
        , _end(data+len)
        , _isspace(boost::bind(&std::isspace<char>, _1, _loc))
        , _isgraph(boost::bind(&std::isgraph<char>, _1, _loc))
    {
    }

    void extractName(Entry& e) {
        if (*_pos != SEQ_BEGIN_CHAR) {
            throw runtime_error(str(format(
                "Fasta sequence line begins with '%1%', expected '%2%"
                ) %*_pos %SEQ_BEGIN_CHAR));
        }
        ++_pos;

        char const* space = find_if(_pos, _end, _isspace);
        e.name = string(_pos, space);
        // skip comment
        _pos = find(_pos, _end, '\n');
        _pos = find_if(_pos, _end, _isgraph);
    }

    void countLines(Entry& e) {
        if (*_pos == SEQ_BEGIN_CHAR) {
            throw runtime_error(str(format(
                "Empty sequence '%1%' in fasta file") %e.name));
        }

        char const* newline = find_if(_pos, _end, _isspace);
        e.len = e.lineLength = e.lineBasesLength = newline - _pos;
        _pos = newline;
        while (_isspace(*_pos)) {
            ++e.lineLength;
            ++_pos;
        }

        while (*_pos != SEQ_BEGIN_CHAR) {
            char const* newline = find_if(_pos, _end, _isspace);
            size_t len = newline - _pos;
            e.len += len;
            _pos = find_if(newline, _end, _isgraph);
            if (len != e.lineBasesLength)
                break;
        }
        // we should be at the next seq now
        if (_pos != _end && *_pos != SEQ_BEGIN_CHAR) {
            throw runtime_error("Uneven line length");
        }
    }

    Fasta::Index* generate() {
        Fasta::Index* index(new Fasta::Index);
        while (_pos < _end) {
            Entry e;
            extractName(e);
            e.offset = _pos - _beg;
            countLines(e);
            index->_entryNames.push_back(e.name);
            index->_entries[e.name] = e;
        }
        return index;
    }

protected:
    char const* _beg;
    char const* _pos;
    char const* _end;
    locale _loc;
    boost::function<bool(char)> _isspace;
    boost::function<bool(char)> _isgraph;
};

Fasta::Fasta(
        std::string const& name,
        char const* data,
        size_t len)
    : _name(name)
    , _index(0)
    , _data(data)
    , _len(len)
{
    IndexGenerator gen(data, len);
    _index = gen.generate();
}

Fasta::Fasta(std::string const& path)
    : _name(path)
    , _index(0)
{
    try {
        _f.reset(new boost::iostreams::mapped_file_source(path));
    } catch (exception const& e) {
        throw IOError(str(format("Failed to memory map fasta '%1%': %2%") %path %e.what()));
    }

    _data = _f->data();
    _len = _f->size();

    string faiPath = path + INDEX_EXTENSION;
    ifstream in(faiPath);
    if (in) {
        _index = new Index(in);
    } else {
        IndexGenerator gen(_data, _len);
        _index = gen.generate();
        ofstream out(faiPath);
        if (!out) {
            throw IOError(str(format(
                "Failed to save fasta index to %1%") %faiPath));
        }
        _index->save(out);
    }
}

Fasta::~Fasta() {
    delete _index;
    _index = 0;
}

std::string const& Fasta::name() const {
    return _name;
}

size_t Fasta::seqlen(std::string const& seq) const {
    Index::Entry const* e = _index->entry(seq);
    if (e) {
        return e->len;
    }

    return 0;
}

std::string Fasta::sequence(std::string const& seq, size_t pos, size_t len) const {
    if (pos == 0) {
        throw runtime_error("Fasta::sequence expects one based coordinates.");
    }

    Index::Entry const* e = _index->entry(seq);
    if (!e) {
        throw UnknownSequenceError(str(format(
            "Sequence '%1%' not found in fasta '%2%'") %seq %_name));
    }

    if (pos > e->len || pos+len - 1 > e->len) {
        throw length_error(str(format(
            "Request for %1%:%2%-%3% in %4%, but %1% has length %5%"
            ) %seq %pos %(pos+len) %_name %e->len));
    }

    // convert to 0 based array index
    --pos;

    size_t skipLines = pos / e->lineBasesLength;
    size_t lineStart = e->offset + skipLines*e->lineLength;
    size_t skipBases = pos - skipLines*e->lineBasesLength;
    string rv;
    while (len) {
        size_t start = lineStart+skipBases;
        size_t thisLen = min(len, e->lineBasesLength-skipBases);
        len -= thisLen;
        rv.append(_data+start, thisLen);
        skipBases = 0;
        lineStart += e->lineLength;
    }

    return rv;
}

char Fasta::sequence(std::string const& seq, size_t pos) const {
    return sequence(seq, pos, 1)[0];
}
