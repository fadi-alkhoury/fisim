/*
 * loanpayoff_test.cc
 *
 *  Created on: Apr 15, 2019
 *      Author: me
 */

#include "loanpayoff.h"

#include "math.h"
#include <gtest/gtest.h>

namespace fisim {

TEST(LoanPayoffTest, payOffPenalty) {
    loan::LoanState loanState;
    loanState.amount = 1000.0;
    loanState.amountInit = 1000.0;
    loanState.interestPaid = 0;
    auto loan = loan::Loan(std::move(loanState));

    rate::RateFixed rateInterest = rate::RateFixed(0.01);
    rate::RateFixed ratePrinciple = rate::RateFixed(0.02);

    // testing payOffPenalty amount and savings with 10% fixed penalty in the first 5 months
    loanhandler::ParamsPayoff paramsPayoff;
    paramsPayoff.penaltyFactor = 0.1;
    paramsPayoff.nMonthsThresh_factorPenalty = 5;
    paramsPayoff.nMonthsPenalty = 0;
    paramsPayoff.nMonthsThresh_nMonthsPenalty = 0;
    paramsPayoff.isFixedNmonthsPenalty = true;
    auto loanPayoff = loanhandler::LoanPayoff(std::move(paramsPayoff));

    tUint nMonthsLoan = 10;
    tAmount totalInterestCost = (1000.0 * 0.01 * (1 - pow(0.98, nMonthsLoan)) / (1 - 0.98));
    tAmount interestPaid = 0;
    tAmount interestOutstanding = 0;
    loanhandler::tPenaltyInfo payoffInfo{0.0, 0.0};
    for (auto nMonthsDone = 0; nMonthsDone < 5; nMonthsDone++) {
        payoffInfo = loanPayoff.payOffPenalty(loan.amountNow(), nMonthsDone, nMonthsLoan, rateInterest, ratePrinciple);
        EXPECT_DOUBLE_EQ(payoffInfo.amount, 0.1 * loan.amountNow());

        interestPaid = (1000.0 * 0.01 * (1 - pow(0.98, nMonthsDone)) / (1 - 0.98)); // interest paid before nMonthsDone
        interestOutstanding = totalInterestCost - interestPaid;
        EXPECT_DOUBLE_EQ(payoffInfo.amountSaved, interestOutstanding - payoffInfo.amount);
        loan.payOnlyPrinciple(0.02 * loan.amountNow());
    }
    for (auto nMonthsDone = 5; nMonthsDone < 15; nMonthsDone++) {
        payoffInfo = loanPayoff.payOffPenalty(loan.amountNow(), nMonthsDone, nMonthsLoan, rateInterest, ratePrinciple);
        EXPECT_DOUBLE_EQ(payoffInfo.amount, 0.01 * loan.amountNow());
    }

    // testing payOffPenalty amount and savings with 3 months penalty in the first 5 months
    loanhandler::ParamsPayoff paramsPayoff1;
    paramsPayoff1.penaltyFactor = 0;
    paramsPayoff1.nMonthsThresh_factorPenalty = 5;
    paramsPayoff1.nMonthsPenalty = 3;
    paramsPayoff1.nMonthsThresh_nMonthsPenalty = 5;
    paramsPayoff1.isFixedNmonthsPenalty = true;
    auto loanPayoff1 = loanhandler::LoanPayoff(std::move(paramsPayoff1));

    loan.payOnlyPrinciple(-1000 + loan.amountNow()); // set to 1000
    for (auto nMonthsDone = 0; nMonthsDone < 5; nMonthsDone++) {
        payoffInfo = loanPayoff1.payOffPenalty(loan.amountNow(), nMonthsDone, nMonthsLoan, rateInterest, ratePrinciple);
        EXPECT_DOUBLE_EQ(payoffInfo.amount, 1000 * 0.01 + 1000 * 0.98 * 0.01 + 1000 * 0.98 * 0.98 * 0.01);
    }
    payoffInfo = loanPayoff1.payOffPenalty(loan.amountNow(), 5, nMonthsLoan, rateInterest, ratePrinciple);
    EXPECT_DOUBLE_EQ(payoffInfo.amount, 0);

    // testing unfixed months penalty -- i.e. months count down
    loanhandler::ParamsPayoff paramsPayoff2;
    paramsPayoff2.penaltyFactor = 0;
    paramsPayoff2.nMonthsThresh_factorPenalty = 5;
    paramsPayoff2.nMonthsPenalty = 3;
    paramsPayoff2.nMonthsThresh_nMonthsPenalty = 5;
    paramsPayoff2.isFixedNmonthsPenalty = false;
    auto loanPayoff2 = loanhandler::LoanPayoff(std::move(paramsPayoff2));
    payoffInfo = loanPayoff2.payOffPenalty(loan.amountNow(), 0, nMonthsLoan, rateInterest, ratePrinciple);
    EXPECT_DOUBLE_EQ(payoffInfo.amount, 1000 * 0.01 + 1000 * 0.98 * 0.01 + 1000 * 0.98 * 0.98 * 0.01);

    loan.payOnlyPrinciple(1000 * 0.02);
    payoffInfo = loanPayoff2.payOffPenalty(loan.amountNow(), 1, nMonthsLoan, rateInterest, ratePrinciple);
    EXPECT_DOUBLE_EQ(payoffInfo.amount, 1000 * 0.98 * 0.01 + 1000 * 0.98 * 0.98 * 0.01);

    loan.payOnlyPrinciple(1000 * 0.98 * 0.02);
    payoffInfo = loanPayoff2.payOffPenalty(loan.amountNow(), 2, nMonthsLoan, rateInterest, ratePrinciple);
    EXPECT_DOUBLE_EQ(payoffInfo.amount, 1000 * 0.98 * 0.98 * 0.01);

    loan.payOnlyPrinciple(1000 * 0.98 * 0.98 * 0.02);
    payoffInfo = loanPayoff2.payOffPenalty(loan.amountNow(), 3, nMonthsLoan, rateInterest, ratePrinciple);
    EXPECT_DOUBLE_EQ(payoffInfo.amount, 0);
}

TEST(LoanPayoffTest, processPayOff) {
    loan::LoanState loanState;
    loanState.amount = 1000.0;
    loanState.amountInit = 1000.0;
    loanState.interestPaid = 0;
    auto loan = loan::Loan(std::move(loanState));

    rate::RateFixed rateInterest = rate::RateFixed(0.01);
    rate::RateFixed ratePrinciple = rate::RateFixed(0.02);

    loanhandler::ParamsPayoff paramsPayoff;
    paramsPayoff.penaltyFactor = 0.1;
    paramsPayoff.nMonthsThresh_factorPenalty = 5;
    paramsPayoff.nMonthsPenalty = 0;
    paramsPayoff.nMonthsThresh_nMonthsPenalty = 0;
    paramsPayoff.isFixedNmonthsPenalty = true;
    auto loanPayoff = loanhandler::LoanPayoff(std::move(paramsPayoff));

    // testing the processPayOff
    tUint nMonthsLoan = 10;
    auto penaltyInfo = loanPayoff.payOffPenalty(loan.amountNow(), 0, nMonthsLoan, rateInterest, ratePrinciple);
    auto result = loanPayoff.processPayOff(loan, penaltyInfo.amount);
    EXPECT_EQ(loan.isPaidOff(), true);
    EXPECT_DOUBLE_EQ(loan.amountNow(), 0);
    EXPECT_DOUBLE_EQ(result.principle, 1000.0);
    EXPECT_DOUBLE_EQ(result.interest, penaltyInfo.amount);
}

} // namespace fisim
