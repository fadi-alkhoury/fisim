#include "types.h"
#include "uncertain_amount.h"
#include "utils_data.h"
#include <gtest/gtest.h>
#include <vector>

using CalendarMonth = fisim::CalendarMonth;
using tAmount = fisim::tAmount;
using tFloat = fisim::tFloat;
using tUint = fisim::tUint;
using tInt = fisim::tInt;
using tSize = fisim::tSize;

TEST(CalendarMonth, Increment) {
    CalendarMonth month = CalendarMonth::jan;

    month = incrementMonth(month, 1);
    EXPECT_EQ(month, CalendarMonth::feb);
    month = incrementMonth(month, 1);
    EXPECT_EQ(month, CalendarMonth::mar);
    month = incrementMonth(month, 1);
    EXPECT_EQ(month, CalendarMonth::apr);
    month = incrementMonth(month, 1);
    EXPECT_EQ(month, CalendarMonth::may);
    month = incrementMonth(month, 1);
    EXPECT_EQ(month, CalendarMonth::jun);
    month = incrementMonth(month, 1);
    EXPECT_EQ(month, CalendarMonth::jul);
    month = incrementMonth(month, 1);
    EXPECT_EQ(month, CalendarMonth::aug);
    month = incrementMonth(month, 1);
    EXPECT_EQ(month, CalendarMonth::sep);
    month = incrementMonth(month, 1);
    EXPECT_EQ(month, CalendarMonth::oct);
    month = incrementMonth(month, 1);
    EXPECT_EQ(month, CalendarMonth::nov);
    month = incrementMonth(month, 1);
    EXPECT_EQ(month, CalendarMonth::dec);
    month = incrementMonth(month, 1);
    EXPECT_EQ(month, CalendarMonth::jan);
}

TEST(CalendarMonth, add) {
    CalendarMonth month = CalendarMonth::jan;

    EXPECT_EQ(incrementMonth(month, 1), CalendarMonth::feb);
    EXPECT_EQ(incrementMonth(month, 2), CalendarMonth::mar);
    EXPECT_EQ(incrementMonth(month, 3), CalendarMonth::apr);
    EXPECT_EQ(incrementMonth(month, 4), CalendarMonth::may);
    EXPECT_EQ(incrementMonth(month, 5), CalendarMonth::jun);
    EXPECT_EQ(incrementMonth(month, 6), CalendarMonth::jul);
    EXPECT_EQ(incrementMonth(month, 7), CalendarMonth::aug);
    EXPECT_EQ(incrementMonth(month, 8), CalendarMonth::sep);
    EXPECT_EQ(incrementMonth(month, 9), CalendarMonth::oct);
    EXPECT_EQ(incrementMonth(month, 10), CalendarMonth::nov);
    EXPECT_EQ(incrementMonth(month, 11), CalendarMonth::dec);
    EXPECT_EQ(incrementMonth(month, 12), CalendarMonth::jan);
    EXPECT_EQ(incrementMonth(month, 13), CalendarMonth::feb);
    EXPECT_EQ(incrementMonth(month, 14), CalendarMonth::mar);
    EXPECT_EQ(incrementMonth(month, 24), CalendarMonth::jan);
}

TEST(CalendarMonth, subtract) {
    CalendarMonth month = CalendarMonth::jan;

    EXPECT_EQ(incrementMonth(month, -1), CalendarMonth::dec);
    EXPECT_EQ(incrementMonth(month, -2), CalendarMonth::nov);
    EXPECT_EQ(incrementMonth(month, -3), CalendarMonth::oct);
    EXPECT_EQ(incrementMonth(month, -4), CalendarMonth::sep);
    EXPECT_EQ(incrementMonth(month, -5), CalendarMonth::aug);
    EXPECT_EQ(incrementMonth(month, -6), CalendarMonth::jul);
    EXPECT_EQ(incrementMonth(month, -7), CalendarMonth::jun);
    EXPECT_EQ(incrementMonth(month, -8), CalendarMonth::may);
    EXPECT_EQ(incrementMonth(month, -9), CalendarMonth::apr);
    EXPECT_EQ(incrementMonth(month, -10), CalendarMonth::mar);
    EXPECT_EQ(incrementMonth(month, -11), CalendarMonth::feb);
    EXPECT_EQ(incrementMonth(month, -12), CalendarMonth::jan);
    EXPECT_EQ(incrementMonth(month, -13), CalendarMonth::dec);
    EXPECT_EQ(incrementMonth(month, -14), CalendarMonth::nov);
    EXPECT_EQ(incrementMonth(month, -24), CalendarMonth::jan);
}

TEST(GrowableAmount, grow) {
    tAmount amount = 100;
    tFloat changeFactor = 1.5;
    tUint changeInterval = 5;
    tInt intervalElapsedInit = 4;

    auto pParams = std::make_unique<fisim::UncertainAmount::Params>();
    pParams->amountMin = amount;
    pParams->amountMax = amount;
    pParams->changeFactorMin = changeFactor;
    pParams->changeFactorMax = changeFactor;
    pParams->changeInterval = changeInterval;
    pParams->intervalElapsedInit = intervalElapsedInit;

    fisim::UncertainAmount growable{std::move(pParams)};

    EXPECT_DOUBLE_EQ(growable.next(), 100);
    for (auto i = 1; i < 6; ++i) {
        EXPECT_DOUBLE_EQ(growable.next(), 150);
    }
    EXPECT_DOUBLE_EQ(growable.next(), 150 * 1.5);
    EXPECT_DOUBLE_EQ(growable.next(), 150 * 1.5);
}

TEST(GrowableAmount, growFutureStart) {
    tAmount amount = 100;
    tFloat changeFactor = 1.5;
    tUint changeInterval = 5;
    tInt intervalElapsedInit = -12; // start 1 year later

    auto pParams = std::make_unique<fisim::UncertainAmount::Params>();
    pParams->amountMin = amount;
    pParams->amountMax = amount;
    pParams->changeFactorMin = changeFactor;
    pParams->changeFactorMax = changeFactor;
    pParams->changeInterval = changeInterval;
    pParams->intervalElapsedInit = intervalElapsedInit;

    fisim::UncertainAmount growable{std::move(pParams)};

    EXPECT_DOUBLE_EQ(growable.next(), 100); // no negative intervals applied
    for (auto i = 1; i < 17; ++i) {
        EXPECT_DOUBLE_EQ(growable.next(), 100);
    }
    EXPECT_DOUBLE_EQ(growable.next(), 150);
}

TEST(Bool, vector) {
    std::vector<fisim::Bool> boolVec;

    boolVec.resize(10u, true);
    boolVec[5u] = false; // construct and then use implicitly defined assignment operator

    for (tSize ind = 0u; ind < 5u; ind++)
        EXPECT_EQ(boolVec[ind], true);
    EXPECT_EQ(boolVec[5u], false);
    for (tSize ind = 6u; ind < 10u; ind++)
        EXPECT_EQ(boolVec[ind], true);

    // test for backdoor access after move
    fisim::Bool* pBackdoor = boolVec.data();

    std::vector<fisim::Bool> boolVec2{std::move(boolVec)};
    EXPECT_EQ(boolVec.size(), 0u);
    EXPECT_EQ(boolVec2.size(), 10u);

    for (tSize ind = 0u; ind < 5u; ind++)
        EXPECT_EQ(boolVec2[ind], true);
    EXPECT_EQ(boolVec2[5u], false);
    for (tSize ind = 6u; ind < 10u; ind++)
        EXPECT_EQ(boolVec2[ind], true);

    for (tSize ind = 0u; ind < 5u; ind++)
        EXPECT_EQ(pBackdoor[ind], true);
    EXPECT_EQ(pBackdoor[5u], false);
    for (tSize ind = 6u; ind < 10u; ind++)
        EXPECT_EQ(pBackdoor[ind], true);
}
