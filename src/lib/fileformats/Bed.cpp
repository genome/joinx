#include "Bed.hpp"
#include "common/Tokenizer.hpp"

#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/tokenizer.hpp>
#include <cstring>

using boost::format;
using namespace std;

void Bed::parseLine(std::string& line, Bed& bed) {
    Tokenizer tokenizer(line);
    if (!tokenizer.extract(bed.chrom))
        throw runtime_error(str(format("Failed to extract chromosome from bed line '%1%'") %line));

    if (!tokenizer.extract(bed.start))
        throw runtime_error(str(format("Failed to extract start position from bed line '%1%'") %line));

    if (!tokenizer.extract(bed.stop))
        throw runtime_error(str(format("Failed to extract stop position from bed line '%1%'") %line));

    if (!tokenizer.extract(bed.refCall))
        throw runtime_error(str(format("Failed to extract ref/call from bed line '%1%'") %line));

    if (!tokenizer.extract(bed.qual))
        throw runtime_error(str(format("Failed to extract quality from bed line '%1%'") %line));

    bed.line.swap(line);
}

int Bed::cmp(const Bed& rhs) const {
    int rv = strverscmp(chrom.c_str(), rhs.chrom.c_str());
    if (rv != 0)
        return rv;

    if (start < rhs.start)
        return -1;
    if (rhs.start < start)
        return 1;

    if (stop < rhs.stop)
        return -1;
    if (rhs.stop < stop)
        return 1;

    return 0;
}

std::ostream& operator<<(std::ostream& s, const Bed& bed) {
    s << bed.line;
    return s;
}
