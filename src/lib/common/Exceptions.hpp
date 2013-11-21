#pragma once

#include <stdexcept>
#include <string>

class CmdlineHelpException : public std::runtime_error {
public:
    CmdlineHelpException(std::string const& msg)
        : std::runtime_error(msg)
    {
    }
};

class IOError : public std::runtime_error {
public:
    explicit IOError(std::string const& message)
        : std::runtime_error(message)
    {
    }
};


class InvalidAlleleError : public std::runtime_error {
public:
    explicit InvalidAlleleError(std::string const& message)
        : std::runtime_error(message)
    {
    }
};
