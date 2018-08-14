set(TARGET_NAME transactions)

# Add a library with the sources
add_library(${TARGET_NAME} 
  src/transacs_recorder.cc)
  
add_library(fisim::transactions ALIAS ${TARGET_NAME})

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
  DESTINATION lib
)
