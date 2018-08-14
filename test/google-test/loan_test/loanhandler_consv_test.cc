/*
 * FISIM_test.cpp
 *
 *  Created on: Sep 28, 2018
 *      Author: me
 */

#include "loan_handler.h"
#include "math.h"
#include "rate.h"
#include <gtest/gtest.h>

namespace fisim {

TEST(LoanHandlerHolTest, base) {
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
    paramsHoliday.nMonthsPerYear = 3;
    paramsHoliday.nMonthsYearRemaining = 0; // DISABLED
    paramsHoliday.nMonthsTermRemaining = 10;
    paramsHoliday.isAllowConsecutive = false;
    paramsHoliday.isSequenceStarted = false;
    paramsHoliday.isExtendTerm = false;
    paramsHoliday.nMonthsStartOffset = 0;

    auto loanHandler = loanhandler::LoanHandler_Hol(std::move(loan), std::move(params), std::move(paramsHoliday));

    loan::tPaymentBreakdown result{0.0, 0.0};

    //---------------------------------------------------------------------------------------//
    // Testing the base class
    //---------------------------------------------------------------------------------------//
    auto interestPaid = 0.0;
    for (auto nMonthsDone = 0; nMonthsDone < 10; nMonthsDone++) {
        result = loanHandler.processMonth(nMonthsDone);
        EXPECT_DOUBLE_EQ(loanHandler.amountNow(), 1000.0 * pow(0.98, nMonthsDone + 1));
        EXPECT_DOUBLE_EQ(result.interest, loanHandler.amountNow() / 0.98 * 0.01);
        EXPECT_DOUBLE_EQ(result.principle, loanHandler.amountNow() / 0.98 * 0.02);
        interestPaid += result.interest;
        EXPECT_DOUBLE_EQ(loanHandler.interestPaid(), interestPaid);
    }

    loanState.amount = 100000.0;
    loanState.amountInit = 1000.0;
    loanState.interestPaid = loanHandler.interestPaid();
    loanhandler::ParamsLoanHandler params1{};
    params1.monthlyPaymentCap = 1000.0;
    std::vector<tFloat> factors{0.01, 0.01, 0.01, 0.0, 0.02};
    params1.rateInterest = std::make_unique<rate::RateVariable>(std::move(factors));
    factors = {0.01, 0.01, 0.01, 0.0, 0.01};
    params1.ratePrinciple = std::make_unique<rate::RateVariable>(std::move(factors));
    auto loanHandler1 = loanhandler::LoanHandler(loan::Loan(std::move(loanState)), std::move(params1));

    result = loanHandler1.processMonth(0);
    EXPECT_DOUBLE_EQ(loanHandler1.amountNow(), 100000.0);
    EXPECT_DOUBLE_EQ(result.interest, 1000);
    EXPECT_DOUBLE_EQ(result.principle, 0);
    interestPaid += result.interest;
    EXPECT_DOUBLE_EQ(loanHandler1.interestPaid(), interestPaid);

    result = loanHandler1.processMonth(4);
    EXPECT_DOUBLE_EQ(loanHandler1.amountNow(), 101000.0);
    EXPECT_DOUBLE_EQ(result.interest, 1000);
    EXPECT_DOUBLE_EQ(result.principle, 0);
    interestPaid += result.interest;
    EXPECT_DOUBLE_EQ(loanHandler1.interestPaid(), interestPaid);

    loanHandler1.processRepayment(loanHandler1.amountNow() - 40000.0); // set amount to 40000
    result = loanHandler1.processMonth(4);
    EXPECT_DOUBLE_EQ(loanHandler1.amountNow(), 40000.0 - 200.0);
    EXPECT_DOUBLE_EQ(result.interest, 800);
    EXPECT_DOUBLE_EQ(result.principle, 200);
    interestPaid += result.interest;
    EXPECT_DOUBLE_EQ(loanHandler1.interestPaid(), interestPaid);

    // negative amortization with the adjustable interest rate loan
    loanHandler1.processRepayment(loanHandler1.amountNow() - 60000.0);
    result = loanHandler1.processMonth(4);
    EXPECT_DOUBLE_EQ(loanHandler1.amountNow(), 60000.0 + 200.0);
    EXPECT_DOUBLE_EQ(result.interest, 1000);
    EXPECT_DOUBLE_EQ(result.principle, 0);
    interestPaid += result.interest;
    EXPECT_DOUBLE_EQ(loanHandler1.interestPaid(), interestPaid);

    // repayment
    loanHandler1.processRepayment(loanHandler1.amountNow() - 201000.0);
    auto amountPaid = loanHandler1.processRepayment(200000);
    EXPECT_DOUBLE_EQ(loanHandler1.amountNow(), 1000.0);
    EXPECT_DOUBLE_EQ(amountPaid, 200000);

    amountPaid = loanHandler1.processRepayment(200000);
    EXPECT_DOUBLE_EQ(loanHandler1.amountNow(), 0.0);
    EXPECT_DOUBLE_EQ(amountPaid, 1000);
}

TEST(LoanHandlerHolTest, processMonth) {
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
    loan::tPaymentBreakdown result{0.0, 0.0};

    // should take a holiday
    result = loanHandler.processMonth(0);
    EXPECT_DOUBLE_EQ(loanHandler.amountNow(), 1000.0);
    EXPECT_DOUBLE_EQ(result.interest, loanHandler.amountNow() * 0.01);
    EXPECT_DOUBLE_EQ(result.principle, 0);
    EXPECT_DOUBLE_EQ(loanHandler.interestPaid(), result.interest);
    auto interestPaid = loanHandler.interestPaid();

    // should not take a holiday -- sequence is not allowed
    result = loanHandler.processMonth(1);
    EXPECT_DOUBLE_EQ(loanHandler.amountNow(), 1000.0 * 0.98);
    EXPECT_DOUBLE_EQ(result.interest, loanHandler.amountNow() / 0.98 * 0.01);
    EXPECT_DOUBLE_EQ(result.principle, 20);
    EXPECT_DOUBLE_EQ(loanHandler.interestPaid(), interestPaid + result.interest);
    interestPaid = loanHandler.interestPaid();

    // should take a holiday -- sequence is reset
    result = loanHandler.processMonth(2);
    EXPECT_DOUBLE_EQ(loanHandler.amountNow(), 1000.0 * 0.98);
    EXPECT_DOUBLE_EQ(result.interest, loanHandler.amountNow() * 0.01);
    EXPECT_DOUBLE_EQ(result.principle, 0);
    EXPECT_DOUBLE_EQ(loanHandler.interestPaid(), interestPaid + result.interest);
    interestPaid = loanHandler.interestPaid();

    // should not take a holiday -- this year's allowance is used up
    result = loanHandler.processMonth(3);
    EXPECT_DOUBLE_EQ(loanHandler.amountNow(), 1000.0 * pow(0.98, 2));
    EXPECT_DOUBLE_EQ(result.interest, loanHandler.amountNow() / 0.98 * 0.01);
    EXPECT_DOUBLE_EQ(result.principle, 1000 * 0.98 * 0.02);
    EXPECT_DOUBLE_EQ(loanHandler.interestPaid(), interestPaid + result.interest);
    interestPaid = loanHandler.interestPaid();

    // should not take a holiday
    result = loanHandler.processMonth(4);
    EXPECT_DOUBLE_EQ(loanHandler.amountNow(), 1000.0 * pow(0.98, 3));
    EXPECT_DOUBLE_EQ(result.interest, loanHandler.amountNow() / 0.98 * 0.01);
    EXPECT_DOUBLE_EQ(result.principle, 1000 * 0.98 * 0.98 * 0.02);
    EXPECT_DOUBLE_EQ(loanHandler.interestPaid(), interestPaid + result.interest);
    interestPaid = loanHandler.interestPaid();

    // should take a holiday -- annual reset
    result = loanHandler.processMonth(12);
    EXPECT_DOUBLE_EQ(loanHandler.amountNow(), 1000.0 * pow(0.98, 3));
    EXPECT_DOUBLE_EQ(result.interest, loanHandler.amountNow() * 0.01);
    EXPECT_DOUBLE_EQ(result.principle, 0);
    EXPECT_DOUBLE_EQ(loanHandler.interestPaid(), interestPaid + result.interest);
}

} // namespace fisim
