set(TARGET_NAME FisimJson)
    
# Add a library with the sources
add_library(${TARGET_NAME} 
  src/json_interface.cc
  src/json_helpers.cc)
  
add_library(fisim::json ALIAS ${TARGET_NAME})

target_include_directories( ${TARGET_NAME}
  PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<INSTALL_INTERFACE:include/fisim> 
)

target_link_libraries(${TARGET_NAME}
  fisim::simulation
  fisim::loan
  fisim::cashflow
  fisim::transactions
  fisim::tax
)



INSTALL(
  TARGETS ${TARGET_NAME}
  EXPORT  export_set
  DESTINATION lib
)


