#############################################
# Unit tests
#############################################

# Add a testing executable
add_executable(common_test common_test.cc)

target_link_libraries(common_test
    fisim::common
    GTest::GTest 
    GTest::Main
)

add_test(test_all common_test)

