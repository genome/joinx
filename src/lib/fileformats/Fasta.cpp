#include "Fasta.hpp"
#include "FastaIndexGenerator.hpp"
#include "common/Exceptions.hpp"
#include "common/Tokenizer.hpp"
#include "common/UnknownSequenceError.hpp"
#include "common/compat.hpp"
#include "io/InputStream.hpp"

#include <boost/bind.hpp>
#include <boost/format.hpp>
#include <boost/function.hpp>
#include <boost/unordered_map.hpp>

#include <algorithm>
#include <cctype>
#include <fstream>
#include <locale>
#include <stdexcept>
#include <vector>

using boost::format;
using namespace std;

namespace {
    const char* INDEX_EXTENSION = ".fai";
}

Fasta::Fasta(
        std::string const& name,
        char const* data,
        size_t len)
    : _name(name)
    , _data(data)
    , _len(len)
{
    FastaIndexGenerator gen(data, len);
    _index = gen.generate();
}

Fasta::Fasta(std::string const& path)
    : _name(path)
{
    try {
        _f = std::make_unique<boost::iostreams::mapped_file_source>(path);
    } catch (exception const& e) {
        throw IOError(str(format("Failed to memory map fasta '%1%': %2%") %path %e.what()));
    }

    _data = _f->data();
    _len = _f->size();

    string faiPath = path + INDEX_EXTENSION;
    ifstream in(faiPath);
    if (in) {
        _index = std::make_unique<FastaIndex>(in);
    } else {
        FastaIndexGenerator gen(_data, _len);
        _index = gen.generate();
        ofstream out(faiPath);
        if (!out) {
            throw IOError(str(format(
                "Failed to save fasta index to %1%") %faiPath));
        }
        _index->save(out);
    }
}

std::string const& Fasta::name() const {
    return _name;
}

size_t Fasta::seqlen(std::string const& seq) const {
    FastaIndex::Entry const* e = _index->entry(seq);
    if (e) {
        return e->len;
    }

    return 0;
}

std::string Fasta::sequence(std::string const& seq, size_t pos, size_t len) const {
    if (pos == 0) {
        throw runtime_error("Fasta::sequence expects one based coordinates.");
    }

    FastaIndex::Entry const* e = _index->entry(seq);
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

FastaIndex const& Fasta::index() const {
    return *_index;
}
