set(TARGET_NAME common)

add_library(${TARGET_NAME} 
  src/types.cc
  src/uncertain_amount.cc
)  
add_library(fisim::common ALIAS ${TARGET_NAME})

target_include_directories( ${TARGET_NAME}
  PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include/fisim> 
    ${CMAKE_INSTALL_PREFIX}/include/
)

find_package(coinorcbc REQUIRED)
target_link_libraries(${TARGET_NAME}
PRIVATE
    libCbc libCgl libOsiClp libOsi libClp libCoinUtils
)

INSTALL(
  TARGETS ${TARGET_NAME}
  EXPORT  export_set
  DESTINATION lib
)
