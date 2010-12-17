#include "bedutil/IResultCollector.hpp"

#include <vector>

class Bed;

class MockResultCollector : public IResultCollector {
public:

    void hit(const Bed& a, const Bed& b) { hitA(a); hitB(b); }
    void hitA(const Bed& a) { _hitA.push_back(a); }
    void hitB(const Bed& b) { _hitB.push_back(b); }
    void missA(const Bed& bed) { _missA.push_back(bed); }
    void missB(const Bed& bed) { _missB.push_back(bed); }

    std::vector<Bed> _hitA;
    std::vector<Bed> _hitB;
    std::vector<Bed> _missA;
    std::vector<Bed> _missB;
};
