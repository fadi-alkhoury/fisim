#############################################
# Unit tests
#############################################

# Add a testing executable
add_executable(monthly_records_serializer_test monthly_records_serializer_test.cc)

target_link_libraries(monthly_records_serializer_test
    fisim::json
    fisim::simulation
    GTest::GTest 
    GTest::Main
)

add_test(test_all monthly_records_serializer_test)

