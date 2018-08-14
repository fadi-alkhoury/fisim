#############################################
# Unit tests
#############################################

# Add a testing executable
add_executable(cashflows_parser_test cashflows_parser_test.cc)

target_link_libraries(cashflows_parser_test
    fisim::json
    fisim::simulation
    GTest::GTest 
    GTest::Main
)

add_test(test_all cashflows_parser_test)

