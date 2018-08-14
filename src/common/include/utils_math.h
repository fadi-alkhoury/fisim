#ifndef FISIM_COMMON_INCLUDE_COMMON_UTIL_MATH_H_
#define FISIM_COMMON_INCLUDE_COMMON_UTIL_MATH_H_

#include "types.h"

#include <cassert>
#include <random>
#include <vector>

namespace fisim {
namespace utils {

///
/// Interpolate the x y series for a specified x value
///
template <typename T, typename W> W interpolate(T x, std::vector<T> const& xPoints, std::vector<W> const& yPoints) {
    assert(xPoints.size() == yPoints.size());
    if (x <= xPoints.front()) {
        return yPoints.front();
    } else if (x >= xPoints.back()) {
        return yPoints.back();
    }

    auto i = 0u;
    while (x > xPoints[i]) {
        ++i;
    }

    auto gradient = (yPoints[i] - yPoints[i - 1]) / ((xPoints[i] - xPoints[i - 1]));
    W y = static_cast<W>(yPoints[i - 1] + gradient * (x - xPoints[i - 1]));

    return y;
}

template <typename floatType = tFloat> floatType randomFactor(floatType a = 0.0, floatType b = 1.0) {
    // use thread_local to make this function thread safe
    thread_local static std::mt19937 mt{std::random_device{}()};
    thread_local static std::uniform_real_distribution<double> dist;

    using pick = std::uniform_real_distribution<double>::param_type;

    return static_cast<floatType>(dist(mt, pick(a, b)));
}

template <typename T> bool isAlmostEqual(T a, T b, T tolerance = 1e-4) {
	return std::abs(a - b) < tolerance;
}

} // namespace utils
} // namespace fisim

#endif /* FISIM_COMMON_INCLUDE_COMMON_UTIL_MATH_H_ */
