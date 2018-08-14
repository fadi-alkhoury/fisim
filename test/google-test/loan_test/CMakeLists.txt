#############################################
# Unit tests
#############################################

# Add a testing executable
add_executable(loan_test 
  loan_test.cc 
  #loanpayoff_test.cc 
  #loanholiday_test.cc 
  #acceleratedpay_test.cc 
  #loanhandler_test.cc
  #loanhandler_agg_test.cc
  #loanhandler_consv_test.cc
  #loancontroller_nor_test.cc  
  #loancontroller_agg_test.cc  
  #loancontroller_consv_test.cc  
  )

target_link_libraries(loan_test
    fisim::loan
    fisim::common
    GTest::GTest 
    GTest::Main
)

add_test(test_all loan_test)

