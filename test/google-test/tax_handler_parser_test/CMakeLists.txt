#############################################
# Unit tests
#############################################

# Add a testing executable
add_executable(tax_handler_parser_test tax_handler_parser_test.cc)

target_link_libraries(tax_handler_parser_test
    fisim::json
    fisim::simulation
    GTest::GTest 
    GTest::Main
)

add_test(test_all tax_handler_parser_test)

