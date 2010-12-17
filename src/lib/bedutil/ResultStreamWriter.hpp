#pragma once

#include "IResultCollector.hpp"

#include <iostream>

class ResultStreamWriter : public IResultCollector {
public:
    ResultStreamWriter(
        std::ostream* hitA,
        std::ostream* hitB,
        std::ostream* missA,
        std::ostream* missB
        );

    void hitA(const Bed& a);
    void hitB(const Bed& b);
    void missA(const Bed& a);
    void missB(const Bed& b);

protected:
    std::ostream* _hitA;
    std::ostream* _hitB;
    std::ostream* _missA;
    std::ostream* _missB;
};
