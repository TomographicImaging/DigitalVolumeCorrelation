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
  find_package(Bash REQUIRED)
  add_test(NAME DATACLOUD_test
         COMMAND ${BASH_EXECUTABLE} run_test.sh
         WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endif()
