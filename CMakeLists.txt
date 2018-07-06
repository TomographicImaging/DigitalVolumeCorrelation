cmake_minimum_required(VERSION 3.0)
project(cildvc)


#find_package (Eigen3 3.3 REQUIRED NO_MODULE)
find_package(OpenMP)

if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    #set (CMAKE_CXX_FLAGS "${OpenMP_CXX_FLAGS}")
    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()


SET (DVC_SRCS ${DVC_SRCS}
	${CMAKE_CURRENT_SOURCE_DIR}/Core/BoundBox.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Core/Cloud.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Core/DataCloud.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Core/FloatingCloud.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Core/InputRead.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Core/Interpolate.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Core/Matrix_4d.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Core/ObjectiveFunctions.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Core/Point.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Core/Search.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Core/SearchParams.cpp
	)
	
add_library(cildvc_static "STATIC" ${DVC_SRCS} )
add_library(cildvc_shared "SHARED" ${DVC_SRCS} )
#TARGET_LINK_LIBRARIES (cilDVC Eigen3::Eigen)
target_include_directories(cildvc_static PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Core/eigen>
	)

target_include_directories(cildvc_shared PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Core/eigen>
	)
add_executable (dvc dvc.cpp )
#TARGET_LINK_LIBRARIES (dvc cilDVC Eigen3::Eigen)
target_link_libraries (dvc cildvc_static)
include_directories(dvc PUBLIC 
	${CMAKE_CURRENT_SOURCE_DIR}/Core )
#INSTALL(TARGETS cilDVC EXPORT CCPiDVCTargets
#        LIBRARY  DESTINATION "${INSTALL_LIB_DIR}" COMPONENT lib
#	PUBLIC_HEADER DESTINATION "${INSTALL_INCLUDE_DIR}" COMPONENT dev
#	RUNTIME  DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
#	ARCHIVE  DESTINATION "${INSTALL_LIB_DIR}" COMPONENT lib
#       )
#INSTALL(TARGETS dvc 
#	RUNTIME  DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
#       )

