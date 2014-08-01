#include "StreamHandler.hpp"

#include "io/GZipLineSource.hpp"
#include "common/Exceptions.hpp"

#include <boost/format.hpp>

#include <cstdio>

using namespace std;
using boost::format;

StreamHandler::StreamHandler()
    : _cinReferences(0)
    , _coutReferences(0)
{
}

std::vector<InputStream::ptr> StreamHandler::openForReading(
        std::vector<std::string> const& paths)
{
    std::vector<InputStream::ptr> rv;
    for (auto i = paths.begin(); i != paths.end(); ++i) {
        rv.push_back(openForReading(*i));
    }
    return rv;
}


InputStream::ptr StreamHandler::openForReading(std::string const& path) {
    ILineSource::ptr lineSource;
    if (path == "-") {
        lineSource.reset(new GZipLineSource(fileno(stdin)));
    }
    else {
        lineSource.reset(new GZipLineSource(path));
    }
    if (!*lineSource) {
        throw IOError(str(format("Failed to open file %1%") %path));
    }
    return InputStream::create(path, lineSource);
}

iostream* StreamHandler::getFile(const std::string& path, openmode mode) {
    auto i = _streams.find(path);
    if (i != _streams.end()) {
        if (mode != i->second.mode || mode == ios::in)
            throw IOError(str(format("Attempted to open file %1% multiple times in an unsupported way. Abort.") %i->first));
        return i->second.stream.get();
    } else {
        Stream s;
        s.stream = boost::shared_ptr<fstream>(new fstream(path.c_str(), mode));
        s.mode = mode;
        if (!*s.stream) {
            throw IOError(str(format("Failed to open file %1%") %path));
        }
        _streams[path] = s;
        return s.stream.get();
    }
}
