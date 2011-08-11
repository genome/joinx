#include "Map.hpp"

#include "common/Tokenizer.hpp"

using boost::format;
using namespace std;

VCF_NAMESPACE_BEGIN

Map::Map() {}

Map::Map(const string& data)
    : _str(data)
{
    if (data.empty())
        return;

    Tokenizer<char> t(data, ',');
    string keyVal;
    while (t.extract(keyVal)) {
        string key;
        Tokenizer<char> eqt(keyVal, '=');
        if (!eqt.extract(key))
            throw runtime_error(str(format("Failed to extract key from string %1% while parsing VCF map %2%") %key %data));

        string value;
        eqt.remaining(value);
        insert(key, value);
    }
}

const std::string& Map::toString() const {
    if (_str.empty()) {
        for (auto i = _keyOrder.begin(); i != _keyOrder.end(); ++i) {
            if (i != _keyOrder.begin())
                _str += ",";
            auto item = _map.find(*i);
            if (item == _map.end())
                continue;

            _str += *i;
            if (!item->second.empty()) {
                _str += "=" + item->second;
            }
        }
    }
    return _str;
}
VCF_NAMESPACE_END
