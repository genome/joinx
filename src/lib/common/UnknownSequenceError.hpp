#pragma once

#include <stdexcept>
#include <string>

class UnknownSequenceError : public std::runtime_error {
public:
    explicit UnknownSequenceError(std::string const& message)
        : std::runtime_error(message)
    {
    }
};
