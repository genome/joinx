#pragma once

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/count.hpp>
#include <boost/accumulators/statistics/max.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/variance.hpp>

#include <cmath>
#include <memory>
#include <vector>
#include <string>
#include <utility>

namespace detail {
    namespace ba = boost::accumulators;

    typedef ba::accumulator_set<
          double
        , ba::stats<
              ba::tag::count
            , ba::tag::mean
            , ba::tag::max
            , ba::tag::variance
            >
        > accum_type;
}

template<
          typename ValueType
        , typename OutputFunc
        >
class GroupStats {
public:
    typedef std::unique_ptr<ValueType> ValuePtr;
    typedef std::vector<ValuePtr> ValuePtrVector;

    GroupStats(OutputFunc& out, std::string name)
        : out_(out)
        , name_(name)
    {}

    void operator()(ValuePtrVector entries) {
        accum_(entries.size());
        out_(std::move(entries));
    }

    template<typename OS>
    friend OS& operator<<(OS& os, GroupStats const& stats) {
        namespace ba = boost::accumulators;
        os << "Group size statistics for '" << stats.name_ << "':\n";
        os << "\tcount: " << ba::count(stats.accum_) << "\n";
        os << "\tmax  : " << ba::max(stats.accum_) << "\n";
        os << "\tmean : " << ba::mean(stats.accum_) << "\n";
        os << "\tsd   : " << std::sqrt(ba::variance(stats.accum_)) << "\n";

        return os;
    }

private:
    OutputFunc& out_;
    std::string name_;
    detail::accum_type accum_;
};

template<
          typename ValueType
        , typename OutputFunc
        >
GroupStats<ValueType, OutputFunc>
makeGroupStats(OutputFunc& out, std::string name) {
    return GroupStats<ValueType, OutputFunc>(out, std::move(name));
}
