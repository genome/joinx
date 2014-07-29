#pragma once

#include "ui/CommandBase.hpp"

#include <cstddef>
#include <string>

class VcfFilterHomopolymersCommand : public CommandBase {
public:
    VcfFilterHomopolymersCommand();

    std::string name() const { return "vcf-filter-homopolymers"; }
    std::string description() const {
        return "filter small indels in homopolymer regions";
    }

    void configureOptions();
    void exec();

protected:
    std::string homopolymerBedFile_;
    std::string infoFieldName_;
    std::string vcfFile_;
    std::string outputFile_;
    size_t maxLength_;
};

