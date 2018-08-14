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

TEST(LoanHandlerAccTest, base) {
    loan::LoanState loanState;
    loanState.amount = 1000.0;
    loanState.amountInit = 1000.0;
    loanState.interestPaid = 0;
    auto loan = loan::Loan(std::move(loanState));

    loanhandler::ParamsLoanHandler params{};
    params.monthlyPaymentCap = 1000.0;
    params.rateInterest = std::make_unique<rate::RateFixed>(0.01);
    params.ratePrinciple = std::make_unique<rate::RateFixed>(0.02);

    loanhandler::ParamsAccelerate paramsAcc;
    paramsAcc.annualAllowanceFactor = 0.1;
    paramsAcc.thisYearRemaining = 100;
    paramsAcc.nMonthsStartOffset = 0;
    paramsAcc.isAnnualReset = true;
    paramsAcc.minAmount = 50;
    paramsAcc.monthChoices = {false, false, true, true, false, true, false, false};
    paramsAcc.isFactorOfStartingAmount = true;

    loanhandler::ParamsPayoff paramsPayoff;
    paramsPayoff.nMonthsPenalty = 0;

    auto loanHandler = loanhandler::LoanHandler_Acc(std::move(loan), std::move(params), std::move(paramsAcc), std::move(paramsPayoff));
    loan::tPaymentBreakdown result = {0.0, 0.0};

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
    params.monthlyPaymentCap = 1000.0;
    std::vector<tFloat> factors{0.01, 0.01, 0.01, 0.0, 0.02};
    params.rateInterest = std::make_unique<rate::RateVariable>(std::move(factors));
    factors = {0.01, 0.01, 0.01, 0.0, 0.01};
    params.ratePrinciple = std::make_unique<rate::RateVariable>(std::move(factors));
    auto loanHandler1 = loanhandler::LoanHandler(loan::Loan(std::move(loanState)), std::move(params));

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

TEST(LoanHandlerAccTest, processMonth) {
    loan::LoanState loanState;
    loanState.amount = 1000.0;
    loanState.amountInit = 1000.0;
    loanState.interestPaid = 0;

    loanhandler::ParamsLoanHandler params{};
    params.monthlyPaymentCap = 1000.0;
    params.rateInterest = std::make_unique<rate::RateFixed>(0.01);
    params.ratePrinciple = std::make_unique<rate::RateFixed>(0.02);

    loanhandler::ParamsAccelerate paramsAcc;
    paramsAcc.annualAllowanceFactor = 0.1;
    paramsAcc.thisYearRemaining = 100;
    paramsAcc.nMonthsStartOffset = 0;
    paramsAcc.isAnnualReset = true;
    paramsAcc.minAmount = 50;
    paramsAcc.monthChoices = {false, false, true, true, false, true, false, false, true, false, false, true,
                              true,  false, true, true, false, true, false, false, true, false, false, true};
    paramsAcc.isFactorOfStartingAmount = true;

    loanhandler::ParamsPayoff paramsPayoff;
    paramsPayoff.nMonthsPenalty = 0;

    auto loanHandler = loanhandler::LoanHandler_Acc(loan::Loan(std::move(loanState)), std::move(params), std::move(paramsAcc), std::move(paramsPayoff));

    auto interestPaid = 0.0;
    loan::tPaymentBreakdown result = {0.0, 0.0};

    // unscheduled payment should NOT be made
    for (auto nMonthsDone = 0; nMonthsDone < 2; nMonthsDone++) {
        result = loanHandler.processMonth(nMonthsDone, 1000);
        EXPECT_DOUBLE_EQ(loanHandler.amountNow(), 1000.0 * pow(0.98, nMonthsDone + 1));
        EXPECT_DOUBLE_EQ(result.interest, loanHandler.amountNow() / 0.98 * 0.01);
        EXPECT_DOUBLE_EQ(result.principle, loanHandler.amountNow() / 0.98 * 0.02);
        interestPaid += result.interest;
        EXPECT_DOUBLE_EQ(loanHandler.interestPaid(), interestPaid);
    }

    // unscheduled payment should be made
    result = loanHandler.processMonth(2, 500);
    EXPECT_DOUBLE_EQ(loanHandler.amountNow(), 1000.0 * pow(0.98, 3) - 100);
    EXPECT_DOUBLE_EQ(result.interest, (loanHandler.amountNow() + 100) / 0.98 * 0.01);
    EXPECT_DOUBLE_EQ(result.principle, (loanHandler.amountNow() + 100) / 0.98 * 0.02 + 100);
    interestPaid += result.interest;
    EXPECT_DOUBLE_EQ(loanHandler.interestPaid(), interestPaid);

    // annual allowance is now used up
    result = loanHandler.processMonth(3, 500);
    EXPECT_DOUBLE_EQ(loanHandler.amountNow(), (1000.0 * pow(0.98, 3) - 100) * 0.98);
    EXPECT_DOUBLE_EQ(result.interest, loanHandler.amountNow() / 0.98 * 0.01);
    EXPECT_DOUBLE_EQ(result.principle, loanHandler.amountNow() / 0.98 * 0.02);
    interestPaid += result.interest;
    EXPECT_DOUBLE_EQ(loanHandler.interestPaid(), interestPaid);

    loanHandler.processRepayment(loanHandler.amountNow() - 1000.0); // set amount to 1000

    // unscheduled payment should be made since allowance is reset
    result = loanHandler.processMonth(12, 500);
    EXPECT_DOUBLE_EQ(loanState.amountInit, 1000.0);
    EXPECT_DOUBLE_EQ(loanHandler.amountNow(), 1000.0 * 0.98 - 100);
    EXPECT_DOUBLE_EQ(result.interest, (loanHandler.amountNow() + 100) / 0.98 * 0.01);
    EXPECT_DOUBLE_EQ(result.principle, (loanHandler.amountNow() + 100) / 0.98 * 0.02 + 100);
    interestPaid += result.interest;
    EXPECT_DOUBLE_EQ(loanHandler.interestPaid(), interestPaid);
}

TEST(LoanHandlerAccTest, processPayOffOption) {
    loan::LoanState loanState;
    loanState.amount = 1000.0;
    loanState.amountInit = 1000.0;
    loanState.interestPaid = 0;
    auto loan = loan::Loan(std::move(loanState));

    loanhandler::ParamsLoanHandler params{};
    params.monthlyPaymentCap = 1000.0;
    params.rateInterest = std::make_unique<rate::RateFixed>(0.01);
    params.ratePrinciple = std::make_unique<rate::RateFixed>(0.02);

    loanhandler::ParamsAccelerate paramsAcc;
    paramsAcc.annualAllowanceFactor = 0.1;
    paramsAcc.thisYearRemaining = 100;
    paramsAcc.nMonthsStartOffset = 0;
    paramsAcc.isAnnualReset = true;
    paramsAcc.minAmount = 50;
    paramsAcc.monthChoices = {};
    paramsAcc.isFactorOfStartingAmount = true;

    loanhandler::ParamsPayoff paramsPayoff;
    paramsPayoff.penaltyFactor = 0.10;
    paramsPayoff.nMonthsThresh_factorPenalty = 5;
    paramsPayoff.nMonthsPenalty = 0;
    paramsPayoff.nMonthsThresh_nMonthsPenalty = 0;
    paramsPayoff.isFixedNmonthsPenalty = true;

    auto loanHandler = loanhandler::LoanHandler_Acc(std::move(loan), std::move(params), std::move(paramsAcc), std::move(paramsPayoff));

    loan::tPaymentBreakdown result = {0.0, 0.0};

    // a payoff should not be triggered -- penalty costs more than not paying off
    result = loanHandler.processPayOffOption(3, 2000, 5);
    EXPECT_DOUBLE_EQ(loanHandler.amountNow(), 1000);
    EXPECT_DOUBLE_EQ(result.interest, 0);
    EXPECT_DOUBLE_EQ(result.principle, 0);

    // a payoff should not be triggered -- penalty costs the same as not paying off
    result = loanHandler.processPayOffOption(3, 2000, 10);
    EXPECT_DOUBLE_EQ(loanHandler.amountNow(), 1000);
    EXPECT_DOUBLE_EQ(result.interest, 0);
    EXPECT_DOUBLE_EQ(result.principle, 0);

    // a payoff should not be triggered -- insufficient amountSpare
    result = loanHandler.processPayOffOption(3, 1099, 100);
    EXPECT_DOUBLE_EQ(loanHandler.amountNow(), 1000);
    EXPECT_DOUBLE_EQ(result.interest, 0);
    EXPECT_DOUBLE_EQ(result.principle, 0);

    // a payoff should be triggered
    result = loanHandler.processPayOffOption(3, 2000, 20);
    EXPECT_DOUBLE_EQ(loanHandler.amountNow(), 0);
    EXPECT_DOUBLE_EQ(result.interest, 100);
    EXPECT_DOUBLE_EQ(result.principle, 1000);

    // a payoff should be triggered -- sufficient amountSpare
    loanState.amount = 1000.0;
    loanState.amountInit = 1000.0;
    loanState.interestPaid = 0;
    loanhandler::ParamsLoanHandler params1{};
    params1.monthlyPaymentCap = 1000.0;
    params1.rateInterest = std::make_unique<rate::RateFixed>(0.01);
    params1.ratePrinciple = std::make_unique<rate::RateFixed>(0.02);

    paramsAcc.annualAllowanceFactor = 0.1;
    paramsAcc.thisYearRemaining = 100;
    paramsAcc.nMonthsStartOffset = 0;
    paramsAcc.isAnnualReset = true;
    paramsAcc.minAmount = 50;
    paramsAcc.monthChoices = {};
    paramsAcc.isFactorOfStartingAmount = true;
    paramsPayoff.penaltyFactor = 0.10;
    paramsPayoff.nMonthsThresh_factorPenalty = 5;
    paramsPayoff.nMonthsPenalty = 0;
    paramsPayoff.nMonthsThresh_nMonthsPenalty = 0;
    paramsPayoff.isFixedNmonthsPenalty = true;

    auto loanHandler1 = loanhandler::LoanHandler_Acc(loan::Loan(std::move(loanState)), std::move(params1), std::move(paramsAcc), std::move(paramsPayoff));

    result = loanHandler1.processPayOffOption(3, 1100, 100);
    EXPECT_DOUBLE_EQ(loanHandler1.amountNow(), 0);
    EXPECT_DOUBLE_EQ(result.interest, 100);
    EXPECT_DOUBLE_EQ(result.principle, 1000);
}

} // namespace fisim
