#############################################
# Unit tests
#############################################

# Add a testing executable
add_executable(transactions_test transactions_test.cc)

target_link_libraries(transactions_test
    fisim::transactions
    GTest::GTest 
    GTest::Main
)

add_test(test_all transactions_test)

