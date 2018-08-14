#############################################
# Unit tests
#############################################

# Add a testing executable
add_executable(tax_test 
	tax_account_test.cc
	tax_handler_test.cc
	)

target_link_libraries(tax_test
    fisim::tax
    GTest::GTest 
    GTest::Main
)

add_test(test_all tax_test)

