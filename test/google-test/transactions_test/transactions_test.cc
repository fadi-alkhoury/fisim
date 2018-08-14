/*
 * FISIM_test.cpp
 *
 *  Created on: Sep 28, 2018
 *      Author: me
 */

#include "transactions_processor.h"
#include <gtest/gtest.h>

///
/// \note These tests neeeds to be updated for the new interfaces. They are excluded in cmake for now.
///

namespace fisim {

TEST(TransactionsTest, add) {
    transactions::TransacsList transacsList;
    transacsList.reserve(10);
    {
        transactions::TransactionsProcessor transacsProcessor{};
        transacsProcessor.startNewList(transacsList);

        Id id{};
        id.idCashflow = 3;
        transacsProcessor.processTransac(id, 10000);
        EXPECT_DOUBLE_EQ(transacsList[0].amount, 10000.0);
        EXPECT_EQ(transacsList[0].id.idCashflow, 3);
        EXPECT_EQ(transacsList.size(), 1);

        id.idCashflow = 30;
        transacsProcessor.processTransac(id, 100000);
        EXPECT_DOUBLE_EQ(transacsList[1].amount, 100000.0);
        EXPECT_DOUBLE_EQ(transacsProcessor.balanceChange(), 110000.0);
        EXPECT_EQ(transacsList[1].id.idCashflow, 30);
        EXPECT_EQ(transacsList.size(), 2);
    } // transacsProcessor out of scope

    EXPECT_DOUBLE_EQ(transacsList[1].amount, 100000.0);
    EXPECT_EQ(transacsList[1].id.idCashflow, 30);
    EXPECT_EQ(transacsList.size(), 2);
}

} // namespace fisim
