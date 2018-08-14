add_compile_options(-Wno-conversion) 
add_compile_options(-Wno-error=conversion) 

# Download and unpack googletest at configure time
# See: http://crascit.com/2015/07/25/cmake-gtest/
configure_file(CMakeLists.txt.in googletest-download/CMakeLists.txt)
# Call CMake to download and Google Test
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
if(result)
  message(FATAL_ERROR "CMake step for googletest failed: ${result}")
endif()
# Build the downloaded google test
execute_process(COMMAND ${CMAKE_COMMAND} --build .
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
if(result)
  message(FATAL_ERROR "Build step for googletest failed: ${result}")
endif()

# Prevent overriding the parent project's compiler/linker
# settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
# Prevent installation of GTest with your project
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
set(INSTALL_GMOCK OFF CACHE BOOL "" FORCE)

# Add googletest directly to our build. This defines
# the gtest and gtest_main targets.
add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/googletest-src
                 ${CMAKE_CURRENT_BINARY_DIR}/googletest-build)

# Add aliases for GTest and GMock libraries
if(NOT TARGET GTest::GTest)
    add_library(GTest::GTest ALIAS gtest)
    add_library(GTest::Main ALIAS gtest_main)
endif()

if(NOT TARGET GTest::GMock)
    add_library(GMock::GMock ALIAS gmock)
    add_library(GMock::Main ALIAS gmock_main)
endif()



add_subdirectory(common_test)
add_subdirectory(optimization_test)
add_subdirectory(loan_test)
add_subdirectory(simulation_test)

# Disabled because these need to be updated to new interfaces
#add_subdirectory(transactions_test)
#add_subdirectory(cashflow_test)
#add_subdirectory(tax_test)

add_subdirectory(utils_parsing_test)
add_subdirectory(tax_handler_parser_test)
add_subdirectory(cashflows_parser_test)
add_subdirectory(loan_controllers_parser_test)
add_subdirectory(monthly_records_serializer_test)

