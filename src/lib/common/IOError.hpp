#pragma once

#include <stdexcept>
#include <string>

class IOError : public std::runtime_error {
public:
    explicit IOError(std::string const& message)
        : std::runtime_error(message)
    {
    }
};
