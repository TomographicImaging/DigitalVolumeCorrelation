#   Copyright 2018 CCPi
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.

#find_package (Eigen3 3.3 REQUIRED NO_MODULE)
find_package(OpenMP)

if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    #set (CMAKE_CXX_FLAGS "${OpenMP_CXX_FLAGS}")
    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

if (WIN32)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /DCCPi_EXPORTS")
endif()




SET (DVC_SRCS ${DVC_SRCS}
	${CMAKE_CURRENT_SOURCE_DIR}/BoundBox.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Cloud.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/DataCloud.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/FloatingCloud.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/InputRead.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Interpolate.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Matrix_4d.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/ObjectiveFunctions.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Point.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Search.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/SearchParams.cpp
	)
	
add_library(cildvc_static "STATIC" ${DVC_SRCS} )
add_library(cildvc_shared "SHARED" ${DVC_SRCS} )
#TARGET_LINK_LIBRARIES (cilDVC Eigen3::Eigen)
target_include_directories(cildvc_static PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/eigen>
	)

target_include_directories(cildvc_shared PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/eigen>
	)
add_executable (dvc ${CMAKE_CURRENT_SOURCE_DIR}/dvc.cpp )
#TARGET_LINK_LIBRARIES (dvc cilDVC Eigen3::Eigen)
target_link_libraries (dvc cildvc_static)
include_directories(dvc PUBLIC 
	${CMAKE_CURRENT_SOURCE_DIR} )

add_executable(parse_header ${CMAKE_CURRENT_SOURCE_DIR}/npy/parse.cpp)
set_property(TARGET parse_header PROPERTY CXX_STANDARD 11)

set_property(TARGET cildvc_static PROPERTY POSITION_INDEPENDENT_CODE ON)

#INSTALL(TARGETS cilDVC EXPORT CCPiDVCTargets
#        LIBRARY  DESTINATION "${INSTALL_LIB_DIR}" COMPONENT lib
#	PUBLIC_HEADER DESTINATION "${INSTALL_INCLUDE_DIR}" COMPONENT dev
#	RUNTIME  DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
#	ARCHIVE  DESTINATION "${INSTALL_LIB_DIR}" COMPONENT lib
#       )

set(INSTALL_BIN_DIR ${CMAKE_INSTALL_PREFIX}/bin)
set(INSTALL_LIB_DIR ${CMAKE_INSTALL_PREFIX}/lib)
set(INSTALL_INCLUDE_DIR ${CMAKE_INSTALL_PREFIX}/include)

install(TARGETS dvc 
	RUNTIME  DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
       )
install(TARGETS cildvc_shared
	LIBRARY  DESTINATION "${INSTALL_LIB_DIR}" COMPONENT lib
	PUBLIC_HEADER DESTINATION "${INSTALL_INCLUDE_DIR}" COMPONENT dev
	RUNTIME  DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
	ARCHIVE  DESTINATION "${INSTALL_LIB_DIR}" COMPONENT lib
	)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/Core/wip 
	DESTINATION ${INSTALL_BIN_DIR})
