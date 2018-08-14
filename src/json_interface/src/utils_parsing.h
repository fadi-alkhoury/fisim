#ifndef INCLUDE_PAYLOAD_PARSER_UTILS_H_
#define INCLUDE_PAYLOAD_PARSER_UTILS_H_

#include "types.h"
#include <cassert>
#include <vector>
#include <array>

namespace fisim {
namespace json {

/** Given a set of x and y points, get an interpolated array of y points for tUint x in [0, xEnd] */
template <typename FloatType, typename SizeType>
std::vector<FloatType> interpolateY(
    std::vector<FloatType> const &pointsY,
    std::vector<SizeType> const &pointsX,
    SizeType xEnd) {
    assert(pointsY.size() == pointsX.size());
    std::size_t nPointsIntrp = xEnd + 1u;
    assert(pointsY.size() < nPointsIntrp);

    std::vector<FloatType> pointsInterpY;
    pointsInterpY.resize(nPointsIntrp);
    auto const nPointsAvailable = pointsY.size();
    auto const xPointsLast = pointsX[nPointsAvailable - 1];
    auto const yPointsLast = pointsY[nPointsAvailable - 1];

    SizeType x = 0;
    auto indPrev = 0u;
    for (auto i = 0u; i < nPointsIntrp; ++i) {
        if (x <= pointsX[0]) {
            pointsInterpY[i] = pointsY[0];
            x++;
        } else if (x >= xPointsLast) {
            pointsInterpY[i] = yPointsLast;
        } else {
            SizeType xPrev = pointsX[indPrev];
            FloatType yPrev = pointsY[indPrev];
            SizeType xNext = pointsX[indPrev + 1u];
            FloatType yNext = pointsY[indPrev + 1u];
            assert(xNext > xPrev);

            auto grad = (yNext - yPrev) / static_cast<FloatType>(xNext - xPrev);
            pointsInterpY[i] = yPrev + grad * static_cast<FloatType>(x - xPrev);

            x++;
            if (x >= xNext && indPrev + 2 < nPointsAvailable) {
                indPrev++;
            }
        }
    }

    return pointsInterpY;
}

std::vector<Bool> calendarToSimMonths(
    std::array<Bool, 12> const &calMonths,
    fisim::CalendarMonth calMonthStart,
    fisim::tUint nMonthsSim) {
    std::vector<Bool> simMonths;
    simMonths.resize(nMonthsSim, false);

    auto calMonth = static_cast<fisim::tUint>(calMonthStart); // 1 for jan
    for (auto i = 0u; i < nMonthsSim; ++i) {
        if (calMonths[calMonth - 1u]) {
            simMonths[i] = true;
        }

        if (calMonth != 12u) {
            calMonth++;
        } else {
            calMonth = 1u;
        }
    }

    return simMonths;
}

} // namespace json
} // namespace fisim

#endif /* INCLUDE_PAYLOAD_PARSER_UTILS_H_ */
