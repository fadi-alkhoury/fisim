/*
 * FISIM_test.cpp
 *
 *  Created on: Sep 28, 2018
 *      Author: me
 */

#include "loan_controller.h"
#include "math.h"
#include <gtest/gtest.h>

namespace fisim {

TEST(LoanControllerLaxTest, processMonthOffset) {
    loan::LoanState loanState;
    loanState.amount = 1000.0;
    loanState.amountInit = 1000.0;
    loanState.interestPaid = 0;
    auto loan = loan::Loan(std::move(loanState));

    loanhandler::ParamsLoanHandler params{};
    params.monthlyPaymentCap = 1000.0;
    params.rateInterest = std::make_unique<rate::RateFixed>(0.01);
    params.ratePrinciple = std::make_unique<rate::RateFixed>(0.02);

    loanhandler::ParamsHoliday paramsHoliday;
    paramsHoliday.nMonthsPerYear = 2;
    paramsHoliday.nMonthsYearRemaining = 2;
    paramsHoliday.nMonthsTermRemaining = 10;
    paramsHoliday.isAllowConsecutive = false;
    paramsHoliday.isSequenceStarted = false;
    paramsHoliday.isExtendTerm = false;
    paramsHoliday.nMonthsStartOffset = 0;

    auto loanHandler = loanhandler::LoanHandler_Hol(std::move(loan), std::move(params), std::move(paramsHoliday));
    tUint nMonthsLoan = 20;
    tUint nMonthsOffset = 1;
    auto loanController = loancontroller::LoanController_Lax(std::move(loanHandler), nMonthsLoan, nMonthsOffset);

    loan::tPaymentBreakdown result{0.0, 0.0};

    // offset -- do nothing
    result = loanController.processMonth(0, 1000);
    EXPECT_DOUBLE_EQ(loanController.amountNow(), 1000.0);
    EXPECT_DOUBLE_EQ(result.interest, 0);
    EXPECT_DOUBLE_EQ(result.principle, 0);
}

TEST(LoanControllerLaxTest, processMonthPaidOff) {
    loan::LoanState loanState;
    loanState.amount = 0.0;
    loanState.amountInit = 1000.0;
    loanState.interestPaid = 0;
    auto loan = loan::Loan(std::move(loanState));

    loanhandler::ParamsLoanHandler params{};
    params.monthlyPaymentCap = 1000.0;
    params.rateInterest = std::make_unique<rate::RateFixed>(0.01);
    params.ratePrinciple = std::make_unique<rate::RateFixed>(0.02);

    loanhandler::ParamsHoliday paramsHoliday;
    paramsHoliday.nMonthsPerYear = 2;
    paramsHoliday.nMonthsYearRemaining = 2;
    paramsHoliday.nMonthsTermRemaining = 10;
    paramsHoliday.isAllowConsecutive = false;
    paramsHoliday.isSequenceStarted = false;
    paramsHoliday.isExtendTerm = false;
    paramsHoliday.nMonthsStartOffset = 0;

    auto loanHandler = loanhandler::LoanHandler_Hol(std::move(loan), std::move(params), std::move(paramsHoliday));
    tUint nMonthsLoan = 20;
    tUint nMonthsOffset = 1;
    auto loanController = loancontroller::LoanController_Lax(std::move(loanHandler), nMonthsLoan, nMonthsOffset);

    loan::tPaymentBreakdown result{0.0, 0.0};

    // loan paid off -- do nothing
    result = loanController.processMonth(1, 1000);
    EXPECT_DOUBLE_EQ(loanController.amountNow(), 0.0);
    EXPECT_DOUBLE_EQ(result.interest, 0);
    EXPECT_DOUBLE_EQ(result.principle, 0);
}

TEST(LoanControllerLaxTest, processMonthHoliday) {
    loan::LoanState loanState;
    loanState.amount = 1000.0;
    loanState.amountInit = 1000.0;
    loanState.interestPaid = 0;
    auto loan = loan::Loan(std::move(loanState));

    loanhandler::ParamsLoanHandler params{};
    params.monthlyPaymentCap = 1000.0;
    params.rateInterest = std::make_unique<rate::RateFixed>(0.01);
    params.ratePrinciple = std::make_unique<rate::RateFixed>(0.02);

    loanhandler::ParamsHoliday paramsHoliday;
    paramsHoliday.nMonthsPerYear = 2;
    paramsHoliday.nMonthsYearRemaining = 2;
    paramsHoliday.nMonthsTermRemaining = 10;
    paramsHoliday.isAllowConsecutive = false;
    paramsHoliday.isSequenceStarted = false;
    paramsHoliday.isExtendTerm = false;
    paramsHoliday.nMonthsStartOffset = 0;

    auto loanHandler = loanhandler::LoanHandler_Hol(std::move(loan), std::move(params), std::move(paramsHoliday));
    tUint nMonthsLoan = 20;
    tUint nMonthsOffset = 1;
    auto loanController = loancontroller::LoanController_Lax(std::move(loanHandler), nMonthsLoan, nMonthsOffset);

    loan::tPaymentBreakdown result{0.0, 0.0};

    // should take a holiday
    loanState.amount = 1000;
    result = loanController.processMonth(2, 1000);
    EXPECT_DOUBLE_EQ(loanController.amountNow(), 1000.0);
    EXPECT_DOUBLE_EQ(result.interest, loanController.amountNow() * 0.01);
    EXPECT_DOUBLE_EQ(result.principle, 0);
    EXPECT_DOUBLE_EQ(loanController.interestPaid(), result.interest);
    auto interesetPaid = result.interest;

    // should not take a holiday -- sequence is not allowed
    loanState.interestPaid = 0;
    result = loanController.processMonth(3, 1000);
    EXPECT_DOUBLE_EQ(loanController.amountNow(), 1000.0 * 0.98);
    EXPECT_DOUBLE_EQ(result.interest, loanController.amountNow() / 0.98 * 0.01);
    EXPECT_DOUBLE_EQ(result.principle, 20);
    EXPECT_DOUBLE_EQ(loanController.interestPaid(), interesetPaid + result.interest);
}

} // namespace fisim
