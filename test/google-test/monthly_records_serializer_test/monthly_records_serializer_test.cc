#include "json_helpers.h"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include "simulator.h"
#include <gtest/gtest.h>

namespace {
using MonthlyRecords = std::vector<fisim::transacs::TransacsRecorder::MonthTransacs>;

TEST(MonthlyRecordSerializer, DISABLED_test00) {
    MonthlyRecords monthlyRecords;
    monthlyRecords.resize(2);

    monthlyRecords[0].balance = 100.0;
    monthlyRecords[0].transacs.resize(10);

    for (auto i = 0u; i < 10; ++i) {
        monthlyRecords[0].transacs[i].amount = i * 40;
        monthlyRecords[0].transacs[i].id.idCashflow = i * 2 + 4;
        monthlyRecords[0].transacs[i].id.idTax = i * 2 + 31;
        monthlyRecords[0].transacs[i].id.idCashflow = i * 3 + 14;
    }

    monthlyRecords[1].balance = 32;
    monthlyRecords[1].transacs.resize(10);
    for (auto i = 0u; i < 10; ++i) {
        monthlyRecords[1].transacs[i].amount = i * 42;
        monthlyRecords[1].transacs[i].id.idCashflow = i * 2 + 42;
        monthlyRecords[1].transacs[i].id.idTax = i * 4 + 1;
        monthlyRecords[1].transacs[i].id.idCashflow = i * 9 + 4;
    }

    // create the json
    rapidjson::StringBuffer strBuffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer{strBuffer};

    fisim::json::monthlyRecordSerializer(monthlyRecords, writer);

    // parse the json and check the values
    auto str = strBuffer.GetString();
    rapidjson::Document document;
    document.Parse(str);

    auto const& doc_monthlyRecords = document["monthlyRecords"].GetArray();
    auto nMonthlyRecords = doc_monthlyRecords.Size();
    EXPECT_EQ(nMonthlyRecords, 2u);

    for (auto i = 0u; i < nMonthlyRecords; ++i) {
        auto& doc_monthTransacs = doc_monthlyRecords[i];
        EXPECT_DOUBLE_EQ(doc_monthTransacs["balance"].GetDouble(), monthlyRecords[i].balance);

        auto const& doc_transacs = doc_monthlyRecords[i]["transacs"].GetArray();
        auto nTransacs = doc_transacs.Size();
        EXPECT_EQ(nTransacs, 10u);

        for (auto indTransac = 0u; indTransac < nTransacs; indTransac++) {
            EXPECT_EQ(doc_transacs[indTransac]["id"]["idCashflow"].GetInt(), monthlyRecords[i].transacs[indTransac].id.idCashflow);
            EXPECT_EQ(doc_transacs[indTransac]["id"]["idLoan"].GetInt(), monthlyRecords[i].transacs[indTransac].id.idLoan);
            EXPECT_EQ(doc_transacs[indTransac]["id"]["idTax"].GetInt(), monthlyRecords[i].transacs[indTransac].id.idTax);

            EXPECT_DOUBLE_EQ(doc_transacs[indTransac]["amount"].GetDouble(), monthlyRecords[i].transacs[indTransac].amount);
        }
    }

} // TEST

} // namespace
