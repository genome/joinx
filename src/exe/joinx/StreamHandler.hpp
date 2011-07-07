#pragma once

#include <iostream>
#include <string>
#include <map>
#include <memory>

class StreamHandler {
public:
    std::iostream* get(const std::string& path, std::ios_base::openmode mode);

protected:
    struct Stream {
        std::shared_ptr<std::iostream> stream;
        std::ios_base::openmode mode;
    };

    std::map<std::string, Stream> _streams;
};
