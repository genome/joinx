#pragma once

#include "ui/CommandBase.hpp"

#include <cstddef>
#include <string>

class VcfAnnotateHomopolymersCommand : public CommandBase {
public:
    VcfAnnotateHomopolymersCommand();

    std::string name() const { return "vcf-annotate-homopolymers"; }
    std::string description() const {
        return "annotate small indels in homopolymer regions";
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

