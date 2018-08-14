set(TARGET_NAME tax)

# Add a library with the sources
add_library(${TARGET_NAME} 
  src/tax_handler.cc
  src/tax_account.cc)
  
add_library(fisim::tax ALIAS ${TARGET_NAME})

target_include_directories( ${TARGET_NAME}
  PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include/fisim> 
)

target_link_libraries(${TARGET_NAME}
  fisim::common
)

INSTALL(
  TARGETS ${TARGET_NAME}
  EXPORT  export_set
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)

