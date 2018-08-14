#############################################
# Unit tests
#############################################

# Add a testing executable
add_executable(loan_controllers_parser_test loan_controllers_parser_test.cc)

target_link_libraries(loan_controllers_parser_test
    fisim::json
    fisim::simulation
    GTest::GTest 
    GTest::Main
)

add_test(test_all loan_controllers_parser_test)

