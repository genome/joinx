#pragma once

#include <iostream>
#include <memory>
#include <string>
#include <vector>

class Bed;

namespace IntersectionOutput {

class ColumnBase;

class Formatter {
public:
    Formatter(const std::string& formatString, std::ostream& s);
    virtual ~Formatter();

    void output(const Bed& a, const Bed& b);
    unsigned extraFields(unsigned which) const;

protected:
    std::string _formatString;
    std::vector<std::unique_ptr<ColumnBase>> _columns;
    std::ostream& _s;
};

}
