#include "StreamHandler.hpp"

#include <boost/format.hpp>

using namespace std;
using boost::format;

StreamHandler::StreamHandler()
    : _cinReferences(0)
    , _coutReferences(0)
{
}

iostream* StreamHandler::getFile(const std::string& path, openmode mode) {
    auto i = _streams.find(path);
    if (i != _streams.end()) {
        if (mode != i->second.mode || mode == ios::in)
            throw runtime_error(str(format("Attempted to open file %1% multiple times in an unsupported way. Abort.") %i->first));
        return i->second.stream.get();
    } else {
        Stream s;
        s.stream = shared_ptr<fstream>(new fstream(path.c_str(), mode));
        s.mode = mode;
        if (!*s.stream) {
            throw runtime_error(str(format("Failed to open file %1%") %path));
        }
        _streams[path] = s;
        return s.stream.get();
    }
}
