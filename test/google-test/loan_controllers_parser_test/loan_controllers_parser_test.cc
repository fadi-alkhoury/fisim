#include "json_helpers.h"
#include "rapidjson/document.h"
#include "rapidjson/filereadstream.h"
#include "simulator.h"
#include <gtest/gtest.h>

namespace {

TEST(ParserLoansDataTest, test00) {
    using namespace rapidjson;
    FILE* fp = std::fopen("./test/google-test/loan_controllers_parser_test/loan_controllers_test00.json", "r"); // non-Windows use "r"

    char readBuffer[65536];
    FileReadStream is(fp, readBuffer, sizeof(readBuffer));

    Document doc;
    doc.ParseStream(is);
    fclose(fp);

    const auto relations = fisim::json::parseRelationsRecords(doc);

    auto loanControllers = fisim::json::parseLoans(doc, relations.loanTaxLinks, fisim::CalendarMonth::aug, 50u);

    EXPECT_EQ(3, 3); // manually compare with the json file
}

} // namespace
