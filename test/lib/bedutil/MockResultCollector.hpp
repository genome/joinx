#include "bedutil/IResultCollector.hpp"

#include <vector>

class Bed;

class MockResultCollector : public IResultCollector {
public:

    void hit(const Bed& a, const Bed& b) { _hit.push_back(a); }
    void missA(const Bed& bed) { _missA.push_back(bed); }
    void missB(const Bed& bed) { _missB.push_back(bed); }

    std::vector<Bed> _hit;
    std::vector<Bed> _missA;
    std::vector<Bed> _missB;
};
