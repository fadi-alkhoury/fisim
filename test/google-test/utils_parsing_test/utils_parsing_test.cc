#include "utils_parsing.h"
#include <gtest/gtest.h>
#include <vector>

namespace {

TEST(UtilsHandler, interpolateYEndAfter) {
    std::vector<fisim::tUint> pointsX{1, 2, 4, 7, 10, 12, 14, 16, 18};
    std::vector<fisim::tFloat> pointsY{1, 2, 4, 7, 10, 14, 22, 10, 0};
    auto xEnd = 19u;

    auto pointsInterpY = fisim::json::interpolateY(pointsY, pointsX, xEnd);

    EXPECT_EQ(pointsInterpY.size(), 20u); // 0 included

    EXPECT_DOUBLE_EQ(pointsInterpY[0], 1);

    for (auto i = 1u; i < 11; ++i) {
        EXPECT_DOUBLE_EQ(pointsInterpY[i], i);
    }

    EXPECT_DOUBLE_EQ(pointsInterpY[11], 12);
    EXPECT_DOUBLE_EQ(pointsInterpY[12], 14);
    EXPECT_DOUBLE_EQ(pointsInterpY[13], 18);
    EXPECT_DOUBLE_EQ(pointsInterpY[14], 22);

    EXPECT_DOUBLE_EQ(pointsInterpY[15], 16);
    EXPECT_DOUBLE_EQ(pointsInterpY[16], 10);

    EXPECT_DOUBLE_EQ(pointsInterpY[17], 5);
    EXPECT_DOUBLE_EQ(pointsInterpY[18], 0);
    EXPECT_DOUBLE_EQ(pointsInterpY[19], 0);
}

TEST(UtilsHandler, interpolateYEndBefore) {
    std::vector<fisim::tUint> pointsX{1, 2, 4, 7, 10, 12, 14, 16, 18};
    std::vector<fisim::tFloat> pointsY{1, 2, 4, 7, 10, 14, 22, 10, 0};
    auto xEnd = 15u;

    auto pointsInterpY = fisim::json::interpolateY(pointsY, pointsX, xEnd);

    EXPECT_EQ(pointsInterpY.size(), 16u); // 0 included

    EXPECT_DOUBLE_EQ(pointsInterpY[0], 1);

    for (auto i = 1u; i < 11; ++i) {
        EXPECT_DOUBLE_EQ(pointsInterpY[i], i);
    }

    EXPECT_DOUBLE_EQ(pointsInterpY[11], 12);
    EXPECT_DOUBLE_EQ(pointsInterpY[12], 14);
    EXPECT_DOUBLE_EQ(pointsInterpY[13], 18);
    EXPECT_DOUBLE_EQ(pointsInterpY[14], 22);

    EXPECT_DOUBLE_EQ(pointsInterpY[15], 16);
}

TEST(UtilsHandler, calendarToSimMonths) {
    std::array<fisim::Bool, 12> calMonths{false, true, false, false, true, false, false, false, false, false, false, false};

    auto calMonthStart = fisim::CalendarMonth::apr;
    auto nMonthsToEnd = 34u;

    auto simMonthsActive = fisim::json::calendarToSimMonths(calMonths, calMonthStart, nMonthsToEnd);

    EXPECT_EQ(simMonthsActive.size(), nMonthsToEnd);

    for (auto i = 0u; i < nMonthsToEnd; ++i) {
        if (i % 12 == 1 || i % 12 == 10) {
            EXPECT_EQ(simMonthsActive[i], true);
        } else {
            EXPECT_EQ(simMonthsActive[i], false);
        }
    }
}

} // namespace
