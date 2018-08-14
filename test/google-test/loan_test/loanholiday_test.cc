/*
 * loanholiday_test.cc
 *
 *  Created on: Apr 15, 2019
 *      Author: me
 */
#include "loanholiday.h"

#include <gtest/gtest.h>

namespace fisim {

TEST(LoanHolidayTest, processHoliday) {
    loan::LoanState loanState;
    loanState.amount = 1000.0;
    loanState.amountInit = 1000.0;
    loanState.interestPaid = 0;
    auto loan = loan::Loan(std::move(loanState));

    loanhandler::ParamsHoliday paramsHoliday;
    paramsHoliday.nMonthsPerYear = 3;
    paramsHoliday.nMonthsYearRemaining = 3;
    paramsHoliday.nMonthsTermRemaining = 10;
    paramsHoliday.isAllowConsecutive = false;
    paramsHoliday.isSequenceStarted = false;
    paramsHoliday.isExtendTerm = false;
    paramsHoliday.nMonthsStartOffset = 0;

    auto loanHoliday = loanhandler::LoanHoliday(std::move(paramsHoliday));

    rate::RateVariable rateInterest = rate::RateVariable({0.01, 0.02, 0.03, 0.04, 0.05});
    rate::RateVariable ratePrinciple = rate::RateVariable({0.01, 0.02, 0.03, 0.04, 0.05});

    tUint nMonthsDone = 0;
    tAmount amountInterest = 100.0;
    tAmount amountInterestPaid = loanHoliday.processHoliday(nMonthsDone, amountInterest, loan, rateInterest, ratePrinciple);
    EXPECT_DOUBLE_EQ(loan.amountNow(), 1000.0);
    EXPECT_DOUBLE_EQ(amountInterestPaid, amountInterest);
    EXPECT_DOUBLE_EQ(loan.interestPaid(), 100);
    EXPECT_EQ(loanHoliday.nMonthsYearRemaining(), 2);
    EXPECT_EQ(loanHoliday.nMonthsTermRemaining(), 9);
    EXPECT_EQ(loanHoliday.nMonthsPerYear(), 3);
    EXPECT_EQ(loanHoliday.isSequenceStarted(), true);

    tUint i = 0;
    for (auto factor : {0.01, 0.02, 0.03, 0.04, 0.05}) { // no change because isExtendTerm is false
        EXPECT_DOUBLE_EQ(rateInterest.rateNow(i), factor);
        EXPECT_DOUBLE_EQ(ratePrinciple.rateNow(i), factor);
        ++i;
    }

    paramsHoliday.nMonthsPerYear = 3;
    paramsHoliday.nMonthsYearRemaining = 2;
    paramsHoliday.nMonthsTermRemaining = 9;
    paramsHoliday.isAllowConsecutive = false;
    paramsHoliday.isSequenceStarted = true;
    paramsHoliday.isExtendTerm = true;
    paramsHoliday.nMonthsStartOffset = 0;

    auto loanHoliday1 = loanhandler::LoanHoliday(std::move(paramsHoliday));
    nMonthsDone = 2;
    amountInterest = 100.0;
    amountInterestPaid = loanHoliday1.processHoliday(nMonthsDone, amountInterest, loan, rateInterest, ratePrinciple);
    EXPECT_DOUBLE_EQ(loan.amountNow(), 1000.0);
    EXPECT_DOUBLE_EQ(amountInterestPaid, amountInterest);
    EXPECT_DOUBLE_EQ(loan.interestPaid(), 200);
    EXPECT_EQ(loanHoliday1.nMonthsYearRemaining(), 1);
    EXPECT_EQ(loanHoliday1.nMonthsTermRemaining(), 8);
    EXPECT_EQ(loanHoliday1.nMonthsPerYear(), 3);
    EXPECT_EQ(loanHoliday1.isSequenceStarted(), true);

    i = 0;
    for (auto factor : {0.01, 0.02, 0.03, 0.03, 0.04, 0.05}) { // element 2 repeated
        EXPECT_DOUBLE_EQ(rateInterest.rateNow(i), factor);
        ++i;
    }
    i = 0;
    for (auto factor : {0.01, 0.02, 0.0, 0.03, 0.04, 0.05}) { // zero inserted
        EXPECT_DOUBLE_EQ(ratePrinciple.rateNow(i), factor);
        ++i;
    }
}

TEST(LoanHolidayTest, isHolidayPossible) {
    loanhandler::ParamsHoliday paramsHoliday;
    paramsHoliday.nMonthsPerYear = 3;
    paramsHoliday.nMonthsYearRemaining = 3;
    paramsHoliday.nMonthsTermRemaining = 10;
    paramsHoliday.isAllowConsecutive = false;
    paramsHoliday.isSequenceStarted = false;
    paramsHoliday.isExtendTerm = false;
    paramsHoliday.nMonthsStartOffset = 0;

    auto loanHoliday = loanhandler::LoanHoliday(std::move(paramsHoliday));

    EXPECT_EQ(loanHoliday.isHolidayPossible(), true);

    paramsHoliday.nMonthsPerYear = 3;
    paramsHoliday.nMonthsYearRemaining = 0;
    paramsHoliday.nMonthsTermRemaining = 10;
    paramsHoliday.isAllowConsecutive = false;
    paramsHoliday.isSequenceStarted = false;
    paramsHoliday.isExtendTerm = false;
    paramsHoliday.nMonthsStartOffset = 0;
    auto loanHoliday1 = loanhandler::LoanHoliday(std::move(paramsHoliday));
    EXPECT_EQ(loanHoliday1.isHolidayPossible(), false);

    paramsHoliday.nMonthsPerYear = 3;
    paramsHoliday.nMonthsYearRemaining = 3;
    paramsHoliday.nMonthsTermRemaining = 10;
    paramsHoliday.isAllowConsecutive = false;
    paramsHoliday.isSequenceStarted = true;
    paramsHoliday.isExtendTerm = false;
    paramsHoliday.nMonthsStartOffset = 0;
    auto loanHoliday2 = loanhandler::LoanHoliday(std::move(paramsHoliday));
    EXPECT_EQ(loanHoliday2.isHolidayPossible(), false);

    paramsHoliday.nMonthsPerYear = 3;
    paramsHoliday.nMonthsYearRemaining = 3;
    paramsHoliday.nMonthsTermRemaining = 10;
    paramsHoliday.isAllowConsecutive = true;
    paramsHoliday.isSequenceStarted = true;
    paramsHoliday.isExtendTerm = false;
    paramsHoliday.nMonthsStartOffset = 0;
    auto loanHoliday3 = loanhandler::LoanHoliday(std::move(paramsHoliday));
    EXPECT_EQ(loanHoliday3.isHolidayPossible(), true);
}

TEST(LoanHolidayTest, isAnnualReset) {
    loanhandler::ParamsHoliday paramsHoliday;
    paramsHoliday.nMonthsPerYear = 3;
    paramsHoliday.nMonthsYearRemaining = 3;
    paramsHoliday.nMonthsTermRemaining = 10;
    paramsHoliday.isAllowConsecutive = false;
    paramsHoliday.isSequenceStarted = false;
    paramsHoliday.isExtendTerm = false;
    paramsHoliday.nMonthsStartOffset = 0;

    auto loanHoliday = loanhandler::LoanHoliday(std::move(paramsHoliday));
    for (auto i = 0u; i < 12; ++i) {
        EXPECT_EQ(loanHoliday.isAnnualReset(i), false);
    }
    for (auto i = 1u; i < 10; ++i) {
        EXPECT_EQ(loanHoliday.isAnnualReset(12 * i), true);
    }
    for (auto i = 13u; i < 24; ++i) {
        EXPECT_EQ(loanHoliday.isAnnualReset(i), false);
    }

    paramsHoliday.nMonthsPerYear = 3;
    paramsHoliday.nMonthsYearRemaining = 3;
    paramsHoliday.nMonthsTermRemaining = 10;
    paramsHoliday.isAllowConsecutive = false;
    paramsHoliday.isSequenceStarted = false;
    paramsHoliday.isExtendTerm = false;
    paramsHoliday.nMonthsStartOffset = 3;

    auto loanHoliday1 = loanhandler::LoanHoliday(std::move(paramsHoliday));
    for (auto i = 0u; i < 9; ++i) {
        EXPECT_EQ(loanHoliday1.isAnnualReset(i), false);
    }
    for (auto i = 1u; i < 10; ++i) {
        EXPECT_EQ(loanHoliday1.isAnnualReset(12 * i - 3), true);
    }
    for (auto i = 10u; i < 21; ++i) {
        EXPECT_EQ(loanHoliday1.isAnnualReset(i), false);
    }
}

TEST(LoanHolidayTest, annualReset) {
    loanhandler::ParamsHoliday paramsHoliday;
    paramsHoliday.nMonthsPerYear = 3;
    paramsHoliday.nMonthsYearRemaining = 1;
    paramsHoliday.nMonthsTermRemaining = 10;
    paramsHoliday.isAllowConsecutive = false;
    paramsHoliday.isSequenceStarted = false;
    paramsHoliday.isExtendTerm = false;
    paramsHoliday.nMonthsStartOffset = 0;
    auto loanHoliday = loanhandler::LoanHoliday(std::move(paramsHoliday));
    loanHoliday.annualReset();
    EXPECT_EQ(loanHoliday.nMonthsYearRemaining(), 3);
    EXPECT_EQ(loanHoliday.nMonthsTermRemaining(), 10);
    EXPECT_EQ(loanHoliday.nMonthsPerYear(), 3);
    EXPECT_EQ(loanHoliday.isSequenceStarted(), false);

    paramsHoliday.nMonthsPerYear = 3;
    paramsHoliday.nMonthsYearRemaining = 1;
    paramsHoliday.nMonthsTermRemaining = 10;
    paramsHoliday.isAllowConsecutive = false;
    paramsHoliday.isSequenceStarted = true;
    paramsHoliday.isExtendTerm = false;
    paramsHoliday.nMonthsStartOffset = 0;
    auto loanHoliday1 = loanhandler::LoanHoliday(std::move(paramsHoliday));
    loanHoliday1.annualReset();
    EXPECT_EQ(loanHoliday1.nMonthsYearRemaining(), 3);
    EXPECT_EQ(loanHoliday1.nMonthsTermRemaining(), 10);
    EXPECT_EQ(loanHoliday1.nMonthsPerYear(), 3);
    EXPECT_EQ(loanHoliday1.isSequenceStarted(), false);

    paramsHoliday.nMonthsPerYear = 2;
    paramsHoliday.nMonthsYearRemaining = 1;
    paramsHoliday.nMonthsTermRemaining = 3;
    paramsHoliday.isAllowConsecutive = false;
    paramsHoliday.isSequenceStarted = false;
    paramsHoliday.isExtendTerm = false;
    paramsHoliday.nMonthsStartOffset = 0;
    auto loanHoliday2 = loanhandler::LoanHoliday(std::move(paramsHoliday));
    loanHoliday2.annualReset();
    EXPECT_EQ(loanHoliday2.nMonthsYearRemaining(), 2);
    EXPECT_EQ(loanHoliday2.nMonthsTermRemaining(), 3);
    EXPECT_EQ(loanHoliday2.nMonthsPerYear(), 2);
    EXPECT_EQ(loanHoliday2.isSequenceStarted(), false);
}

} // namespace fisim
