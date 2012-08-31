#pragma once

#include <stdexcept>
#include <string>

class UnsortedDataError : public std::runtime_error {
public:
    explicit UnsortedDataError(std::string const& message)
        : std::runtime_error(message)
    {
    }
};
