add_executable (tests ${CMAKE_CURRENT_SOURCE_DIR}/tests.cpp )
target_link_libraries (tests cildvc_static)
include_directories(tests PUBLIC ${CMAKE_SOURCE_DIR}/Core )

add_executable(test1 test_DataCloud.cpp)
target_link_libraries (test1 cildvc_static)
include_directories(test1 PUBLIC ${CMAKE_SOURCE_DIR}/Core )


install(TARGETS tests test1
RUNTIME  DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
)


message (WARNING "Adding test test1")

add_test(NAME DATACLOUD_test
           COMMAND test1 datacloud_input.txt
           WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

