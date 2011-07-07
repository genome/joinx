#pragma once

#include <iostream>
#include <string>
#include <map>

class StreamHandler {
public:
    StreamHandler();
    virtual ~StreamHandler();

    std::iostream* get(const std::string& path, std::ios_base::openmode mode);

protected:
    struct Stream {
        std::iostream* stream;
        std::ios_base::openmode mode;
    };

    std::map<std::string, Stream> _streams;
};
