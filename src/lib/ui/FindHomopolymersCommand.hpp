#pragma once

#include "ui/CommandBase.hpp"

#include <array>
#include <cstddef>
#include <string>
#include <vector>

class FindHomopolymersCommand : public CommandBase {
public:
    FindHomopolymersCommand();

    std::string name() const { return "find-homopolymers"; }
    std::string description() const {
        return "find homopolymers of a minimum size or greater in fasta files";
    }

    void configureOptions();
    void finalizeOptions();
    void exec();

protected:
    std::string fasta_;
    std::string outputFile_;
    std::vector<std::string> sequences_;
    std::string ignoreChars_;
    size_t minLength_;
    std::array<bool, 256> ignoreArray_;
};

