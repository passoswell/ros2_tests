# add_subdirectory(minimal_publisher)
# add_subdirectory(minimal_subscriber)


# Add every subdirectory that contains a CMakeLists.txt
file(GLOB children RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/*)

foreach(child ${children})
  if(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${child})
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${child}/CMakeLists.txt)
      add_subdirectory(${child})
    endif()
  endif()
endforeach()