#include "json_interface.h"
#include <fstream>
#include <gtest/gtest.h>

constexpr auto kPrecision = 0.01;

//// 
 /// Test the simulation specified in simulation_test01.json
 /// 
TEST(simulator, test01) {
    std::ifstream fStreamJson{"./test/google-test/simulation_test/simulation_test01.json"};
    const std::string str{std::istreambuf_iterator<char>(fStreamJson), std::istreambuf_iterator<char>()};
    rapidjson::Document document;
    document.Parse(str);

    const auto monthlyRecords = fisim::json::runSim(document);

    EXPECT_EQ(120u, monthlyRecords.size());
    EXPECT_NEAR(282304.34, monthlyRecords.back().balance, kPrecision);
}
