#include "Sequence.hpp"

#include "bedutil/intconfig.hpp"

#include <boost/algorithm/string.hpp>
#include <cassert>

using namespace std;

namespace {
    const uint8_t* translationTable() {
        static bool initialized = false;
        static uint8_t table[255] = {0};
        const static string in ("acgtrymkswhbvdnxACGTRYMKSWHBVDNX");
        const static string out("tgcayrkmswdvbhnxTGCAYRKMSWDVBHNX");
        if (!initialized) {
            assert(in.size() == out.size());
            for (unsigned i = 0; i < 255; ++i)
                table[i] = '-';
            for (string::size_type i = 0; i < in.size(); ++i)
                table[int(in[i])] = out[i];
        }
        return table;
    }
}

Sequence::Sequence() {}

Sequence::Sequence(const std::string& data) : _data(data) {
    boost::to_upper(_data);
}

std::string Sequence::reverseComplement(const std::string& data) {
    std::string rv;
    rv.resize(data.size());
    for (string::size_type i = 0; i < data.size(); ++i)
        rv[i] = translationTable()[int(data[data.size()-1-i])];
    return rv;
}

const std::string& Sequence::reverseComplementData() const {
    if (_reverseComplement.empty() && !_data.empty())
        _reverseComplement = reverseComplement(data());
    return _reverseComplement;
}

