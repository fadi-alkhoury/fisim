/*
 * FISIM_test.cpp
 *
 *  Created on: Sep 28, 2018
 *      Author: me
 */

#include "cashflow.h"
#include <gtest/gtest.h>

namespace fisim {

TEST(cashflowRandomLoss, test00) {
    cashflow::CashflowParams params;

    params.amount = 1000.0;
    params.changeFactor = 1.0;
    params.changeInterval = 12u;
    params.intervalStartOffset = 4u;
    params.stepsActive.reserve(1000000u);
    for (auto i = 0u; i <= 5000000u; ++i) {
        params.stepsActive.push_back(true);
    }
    const auto probability = 0.1f;

    const auto cashflow = cashflow::CashflowRandomLoss(std::move(params), probability);

    cashflow::tCashflowResult result;

    tAmount amountLostMin = params.amount;
    tAmount amountLostMax = 0.0;
    tAmount amountSum = 0.0;
    tAmount amountSumLost = 0.0;
    const auto iterCount = 5000000u;
    for (auto i = 0u; i < iterCount; ++i) {
        result = cashflow.findCashflow(i);
        amountSum += result.amount;
        amountSumLost += result.amountLost;

        if (result.amountLost < amountLostMin) {
            amountLostMin = result.amountLost;
        }
        if (result.amountLost > amountLostMax) {
            amountLostMax = result.amountLost;
        }
    }

    EXPECT_DOUBLE_EQ(amountSum / iterCount, params.amount); // result.amount is always the same
    EXPECT_NEAR(amountSumLost / iterCount, params.amount * probability, 0.20);
    EXPECT_DOUBLE_EQ(amountLostMin, 0.0);
    EXPECT_DOUBLE_EQ(amountLostMax, params.amount);
}

} // namespace fisim
