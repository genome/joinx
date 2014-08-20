#pragma once

#include "RawVariant.hpp"
#include "common/namespaces.hpp"

#include <boost/container/flat_set.hpp>
#include <boost/unordered_map.hpp>

BEGIN_NAMESPACE(Vcf)

template<typename T>
class GenotypeDictionary {
public:
    typedef T ValueType;
    typedef size_t Count;

    typedef boost::container::flat_set<ValueType> Locations;
    typedef boost::unordered_map<Vcf::RawVariant::Vector, Locations> ExactMapType;

    typedef boost::unordered_map<ValueType, Count> LocationCounts;
    typedef boost::unordered_map<Vcf::RawVariant, LocationCounts> PartialMapType;

    void add(Vcf::RawVariant::Vector const& gt, ValueType value);
    void addAlleles(Vcf::RawVariant::Vector const& gt, ValueType value);
    void clear();

    Locations const& exactMatches(Vcf::RawVariant::Vector const& gt) const;
    LocationCounts const& allMatches(Vcf::RawVariant const& gt) const;

    PartialMapType const& partialMatches() const { return partialMap_; }
    ExactMapType const& exactMatches() const { return exactMap_; }

private:
    ExactMapType exactMap_;
    PartialMapType partialMap_;
};

template<typename T>
inline
void GenotypeDictionary<T>::add(Vcf::RawVariant::Vector const& gt, ValueType value) {
    exactMap_[gt].insert(value);
    addAlleles(gt, value);
}

template<typename T>
inline
void GenotypeDictionary<T>::addAlleles(Vcf::RawVariant::Vector const& gt, ValueType value) {
    for (auto x = gt.begin(); x != gt.end(); ++x) {
        ++partialMap_[*x][value];
    }
}

template<typename T>
inline
auto GenotypeDictionary<T>::allMatches(Vcf::RawVariant const& gt) const -> LocationCounts const& {
    static LocationCounts empty;
    auto iter = partialMap_.find(gt);
    if (iter != partialMap_.end()) {
        return iter->second;
    }
    return empty;
}

template<typename T>
inline
auto GenotypeDictionary<T>::exactMatches(Vcf::RawVariant::Vector const& gt) const -> Locations const& {
    static Locations empty;
    auto iter = exactMap_.find(gt);
    if (iter != exactMap_.end()) {
        return iter->second;
    }
    return empty;
}

template<typename T>
inline
void GenotypeDictionary<T>::clear() {
    partialMap_.clear();
    exactMap_.clear();
}

END_NAMESPACE(Vcf)
