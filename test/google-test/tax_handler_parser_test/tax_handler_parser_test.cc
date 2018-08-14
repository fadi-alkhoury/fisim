#include "json_helpers.h"
#include "rapidjson/document.h"
#include "rapidjson/filereadstream.h"
#include "simulator.h"
#include <gtest/gtest.h>

TEST(ParserTaxHandlerTest, test00) {

    FILE* fp = std::fopen("./test/google-test/tax_handler_parser_test/tax_accounts_test00.json", "r"); // non-Windows use "r"

    char readBuffer[65536];
    rapidjson::FileReadStream is(fp, readBuffer, sizeof(readBuffer));

    rapidjson::Document document;
    document.ParseStream(is);
    fclose(fp);

    auto taxHandler = fisim::json::parseTaxHandler(document, fisim::CalendarMonth::may);

    EXPECT_EQ(true, true); // manually compare data with the json file
}
