add_executable (tests ${CMAKE_CURRENT_SOURCE_DIR}/tests.cpp )
target_link_libraries (tests cildvc_static)
include_directories(tests PUBLIC ${CMAKE_SOURCE_DIR}/Core )

add_executable(test1 test_DataCloud.cpp)
target_link_libraries (test1 cildvc_static)
include_directories(test1 PUBLIC ${CMAKE_SOURCE_DIR}/Core )


install(TARGETS tests test1
RUNTIME  DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
)


if (NOT WIN32)
  message (WARNING "Adding test DATACLOUD_test")
  find_program(BASH_PROGRAM NAMES bash)

  if (NOT BASH_PROGRAM)
      message(FATAL_ERROR "Bash executable not found!")
  endif()

  message(STATUS "Found Bash at: ${BASH_PROGRAM}")
  add_custom_target(GENERATE_TESTDATA ALL
    COMMAND ${BASH_PROGRAM} run_test.sh
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  )
  add_test(NAME DATACLOUD_test
         COMMAND test1 datacloud_input.txt
         WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

  add_dependencies(test1 GENERATE_TESTDATA)
endif()
