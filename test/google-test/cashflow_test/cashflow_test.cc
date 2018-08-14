/*
 * FISIM_test.cpp
 *
 *  Created on: Sep 28, 2018
 *      Author: me
 */

#include "cashflow.h"
#include <gtest/gtest.h>

namespace fisim {

///
/// \note These tests neeeds to be updated for the new interfaces. They are excluded in cmake for now.
///

TEST(cashflow, test00) {
    cashflow::CashflowParams params;

    params.amount = 100.0; // base amount, subject to annual multiplier
    params.changeFactor = 1.5;
    params.changeInterval = 12u;
    params.intervalStartOffset = 4u;
    params.stepsActive.reserve(31);
    for (auto i = 0u; i <= 31u; ++i) {
        params.stepsActive.push_back(true);
    }

    auto cashflow = cashflow::Cashflow(std::move(params));

    for (auto i = 0u; i <= 7u; ++i) {
        auto result = cashflow.findCashflow(i);
        EXPECT_DOUBLE_EQ(result.amount, 100.0);
        EXPECT_DOUBLE_EQ(result.amountLost, 0.0);
    }

    for (auto i = 8u; i <= 19u; ++i) {
        auto result = cashflow.findCashflow(i);
        EXPECT_DOUBLE_EQ(result.amount, 150.0);
        EXPECT_DOUBLE_EQ(result.amountLost, 0.0);
    }

    for (auto i = 20u; i <= 31u; ++i) {
        auto result = cashflow.findCashflow(i);
        EXPECT_DOUBLE_EQ(result.amount, 225.0);
        EXPECT_DOUBLE_EQ(result.amountLost, 0.0);
    }

    params.amount = 100.0; // base amount, subject to annual multiplier
    params.changeFactor = 1.5;
    params.changeInterval = 12u;
    params.intervalStartOffset = 10u;
    params.stepsActive.reserve(31);
    for (auto i = 0u; i <= 31u; ++i) {
        params.stepsActive.push_back(true);
    }
    cashflow::Cashflow cashflow2(std::move(params));

    auto result = cashflow2.findCashflow(2u);
    EXPECT_DOUBLE_EQ(result.amount, 150.0);
}

TEST(cashflow, test01) {
    cashflow::CashflowParams params;

    params.amount = 100000.0; // base amount, subject to annual multiplier
    params.changeFactor = 1.0;
    params.intervalStartOffset = 5u;
    params.changeInterval = 12u;
    params.stepsActive.reserve(120);
    for (auto i = 0u; i < 120u; ++i) {
        params.stepsActive.push_back(false);
    }
    for (auto i = 0u; i < 10u; ++i) {
        params.stepsActive[12 * i] = true;
        params.stepsActive[12 * i + 3] = true;
    }

    auto cashflow = cashflow::Cashflow(std::move(params));

    for (auto n = 0; n <= 4; n++) {
        auto result = cashflow.findCashflow(12 * n);
        EXPECT_DOUBLE_EQ(result.amount, 100000.0);
        EXPECT_DOUBLE_EQ(result.amountLost, 0.0);
    }

    for (auto i = 1u; i <= 2u; ++i) {
        auto result = cashflow.findCashflow(i);
        EXPECT_DOUBLE_EQ(result.amount, 0.0);
        EXPECT_DOUBLE_EQ(result.amountLost, 0.0);
    }

    for (auto n = 0; n <= 4; n++) {
        auto result = cashflow.findCashflow(12 * n + 3);
        EXPECT_DOUBLE_EQ(result.amount, 100000.0);
        EXPECT_DOUBLE_EQ(result.amountLost, 0.0);
    }

    for (auto i = 4u; i <= 11u; ++i) {
        auto result = cashflow.findCashflow(i);
        EXPECT_DOUBLE_EQ(result.amount, 0.0);
        EXPECT_DOUBLE_EQ(result.amountLost, 0.0);
    }

    for (auto i = 13u; i <= 14u; ++i) {
        auto result = cashflow.findCashflow(i);
        EXPECT_DOUBLE_EQ(result.amount, 0.0);
        EXPECT_DOUBLE_EQ(result.amountLost, 0.0);
    }

    for (auto n = 0; n <= 4; n++) {
        auto result = cashflow.findCashflow(12 * n + 3);
        EXPECT_DOUBLE_EQ(result.amount, 100000.0);
        EXPECT_DOUBLE_EQ(result.amountLost, 0.0);
    }
}

} // namespace fisim
