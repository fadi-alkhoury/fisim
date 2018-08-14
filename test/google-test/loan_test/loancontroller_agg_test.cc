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

TEST(LoanControllerAggTest, test00) {
    loan::LoanState loanState;
    loanState.amount = 1000.0;
    loanState.amountInit = 1000.0;
    loanState.interestPaid = 0;
    auto loan = loan::Loan(std::move(loanState));

    tAmount monthlyPaymentCap = 1000.0;

    loanhandler::ParamsLoanHandler params{};
    params.monthlyPaymentCap = monthlyPaymentCap;
    params.rateInterest = std::make_unique<rate::RateFixed>(0.01);
    params.ratePrinciple = std::make_unique<rate::RateFixed>(0.02);

    loanhandler::ParamsAccelerate paramsAcc;
    paramsAcc.annualAllowanceFactor = 0.10;
    paramsAcc.thisYearRemaining = 100;
    paramsAcc.nMonthsStartOffset = 0;
    paramsAcc.isAnnualReset = true;
    paramsAcc.minAmount = 50;
    paramsAcc.monthChoices = {false, false, true, true, false, true, false, false, true, false, false, true,
                              true,  false, true, true, false, true, false, false, true, false, false, true};
    paramsAcc.isFactorOfStartingAmount = true;

    loanhandler::ParamsPayoff paramsPayoff;
    paramsPayoff.penaltyFactor = 0.10;
    paramsPayoff.nMonthsThresh_factorPenalty = 5;
    paramsPayoff.nMonthsPenalty = 0;               // irrelevant
    paramsPayoff.nMonthsThresh_nMonthsPenalty = 0; // irrelevant
    paramsPayoff.isFixedNmonthsPenalty = true;

    auto loanHandler = loanhandler::LoanHandler_Acc(std::move(loan), std::move(params), std::move(paramsAcc), std::move(paramsPayoff));
    tUint nMonthsLoan = 20;
    tUint nMonthsOffset = 0;
    auto loanController = loancontroller::LoanController_Agg(std::move(loanHandler), nMonthsLoan, nMonthsOffset);

    loan::tPaymentBreakdown result{0.0, 0.0};

    auto interestPaid = 0.0;
    for (auto nMonthsDone = 0; nMonthsDone < 2; nMonthsDone++) {
        result = loanController.processMonth(nMonthsDone, 1000);
        EXPECT_DOUBLE_EQ(loanController.amountNow(), 1000.0 * pow(0.98, nMonthsDone + 1));
        EXPECT_DOUBLE_EQ(result.interest, loanController.amountNow() / 0.98 * 0.01);
        EXPECT_DOUBLE_EQ(result.principle, loanController.amountNow() / 0.98 * 0.02);
        interestPaid += result.interest;
        EXPECT_DOUBLE_EQ(loanController.interestPaid(), interestPaid);
    }

    // unscheduled payment should be made
    result = loanController.processMonth(2, 500); // not enough funds to trigger a payoff
    EXPECT_DOUBLE_EQ(loanController.amountNow(), 1000.0 * pow(0.98, 3) - 100);
    EXPECT_DOUBLE_EQ(result.interest, (loanController.amountNow() + 100) / 0.98 * 0.01);
    EXPECT_DOUBLE_EQ(result.principle, (loanController.amountNow() + 100) / 0.98 * 0.02 + 100);
    interestPaid += result.interest;
    EXPECT_DOUBLE_EQ(loanController.interestPaid(), interestPaid);

    // annual allowance is now used up
    result = loanController.processMonth(3, 500);
    EXPECT_DOUBLE_EQ(loanController.amountNow(), (1000.0 * pow(0.98, 3) - 100) * 0.98);
    EXPECT_DOUBLE_EQ(result.interest, loanController.amountNow() / 0.98 * 0.01);
    EXPECT_DOUBLE_EQ(result.principle, loanController.amountNow() / 0.98 * 0.02);
    interestPaid += result.interest;
    EXPECT_DOUBLE_EQ(loanController.interestPaid(), interestPaid);
    auto amount = (1000.0 * pow(0.98, 3) - 100) * 0.98;

    result = loanController.processMonth(12, 500); // allowance should be reset
    EXPECT_DOUBLE_EQ(loanController.amountNow(), amount * 0.98 - 100);
    EXPECT_DOUBLE_EQ(result.interest, (loanController.amountNow() + 100) / 0.98 * 0.01);
    EXPECT_DOUBLE_EQ(result.principle, (loanController.amountNow() + 100) / 0.98 * 0.02 + 100);
    interestPaid += result.interest;
    amount = amount * 0.98 - 100;
    EXPECT_DOUBLE_EQ(loanController.interestPaid(), interestPaid);

    // a payoff should be triggered
    result = loanController.processMonth(3, 2000);
    EXPECT_DOUBLE_EQ(loanController.amountNow(), 0);
    EXPECT_DOUBLE_EQ(result.interest, amount / 10.0);
    EXPECT_DOUBLE_EQ(result.principle, amount);
    interestPaid += result.interest;
    EXPECT_DOUBLE_EQ(loanController.interestPaid(), interestPaid);

    // testing EOT
    paramsAcc.annualAllowanceFactor = 0.10;
    paramsAcc.thisYearRemaining = 0;
    paramsAcc.nMonthsStartOffset = 0;
    paramsAcc.isAnnualReset = true;
    paramsAcc.minAmount = 50;
    paramsAcc.monthChoices = {false, false, true, true, false, true, false, false, true, false, false, true,
                              true,  false, true, true, false, true, false, false, true, false, false, true};
    paramsAcc.isFactorOfStartingAmount = true;
    paramsPayoff.penaltyFactor = 0.10;
    paramsPayoff.nMonthsThresh_factorPenalty = 5;
    paramsPayoff.nMonthsPenalty = 0;               // irrelevant
    paramsPayoff.nMonthsThresh_nMonthsPenalty = 0; // irrelevant
    paramsPayoff.isFixedNmonthsPenalty = true;

    loanhandler::ParamsLoanHandler params1{};
    params1.monthlyPaymentCap = monthlyPaymentCap;
    std::vector<tFloat> factors{0.01, 0.01, 0.01, 0.0, 0.02, 0.02};
    params1.rateInterest = std::make_unique<rate::RateVariable>(std::move(factors));
    factors = {0.01, 0.01, 0.01, 0.0, 0.01, 0.01};
    params1.ratePrinciple = std::make_unique<rate::RateVariable>(std::move(factors));
    nMonthsLoan = 6;
    loanState.amount = 1000.0;
    loanState.amountInit = 1000.0;
    loanState.interestPaid = interestPaid;
    auto loan1 = loan::Loan(std::move(loanState));
    auto loanHandler1 = loanhandler::LoanHandler_Acc(std::move(loan1), std::move(params1), std::move(paramsAcc), std::move(paramsPayoff));
    auto loanController1 = loancontroller::LoanController_Agg(std::move(loanHandler1), nMonthsLoan, nMonthsOffset);

    result = loanController1.processMonth(5, 1000); // last month remains
    EXPECT_DOUBLE_EQ(loanController1.amountNow(), 20.0);
    EXPECT_DOUBLE_EQ(result.interest, 20);
    EXPECT_DOUBLE_EQ(result.principle, 10 + 970);
    interestPaid += result.interest;
    EXPECT_DOUBLE_EQ(loanController1.interestPaid(), interestPaid);
}

} // namespace fisim
