#############################################
# Unit tests
#############################################

# Add a testing executable
add_executable(cashflow_test 
  cashflow_test.cc 
  cashflowAverageLoss_test.cc
  cashflowRandomLoss_test.cc
  )

target_link_libraries(cashflow_test
    fisim::cashflow
    fisim::common
    GTest::GTest 
    GTest::Main
)

add_test(test_all cashflow_test)

