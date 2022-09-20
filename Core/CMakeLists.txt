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

if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/CCPiDefines.h)
   file(REMOVE ${CMAKE_CURRENT_SOURCE_DIR}/CCPiDefines.h)
endif()
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CCPiDefines.h.in ${CMAKE_CURRENT_SOURCE_DIR}/CCPiDefines.h)


if (CMAKE_COMPILER_IS_GNUCC AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.7)
    message(FATAL_ERROR "Require at least gcc-4.7")
endif()

find_package(OpenMP REQUIRED)
add_definitions(${OpenMP_CXX_FLAGS})


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

if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.9.0") 
  set (OpenMP_EXE_LINKER_FLAGS OpenMP::OpenMP_CXX)
else()
  message(WARNING "Your CMake version is old. OpenMP linking flags  might be incorrect.")
  # need to explicitly set this. Definitely for gcc, hopefully also for other systems.
  # See https://gitlab.kitware.com/cmake/cmake/issues/15392
  set (OpenMP_EXE_LINKER_FLAGS ${OpenMP_C_FLAGS})
endif()

# add_library(cildvc_shared "SHARED" ${DVC_SRCS} )
# TARGET_LINK_LIBRARIES (cilDVC Eigen3::Eigen)
target_include_directories(cildvc_static PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/eigen>
    $<BUILD_INTERFACE:${OpenMP_CXX_INCLUDE_DIRS}>
)
# target_link_libraries(cildvc_static ${OpenMP_CXX_LIBRARIES})
target_link_libraries(cildvc_static ${OpenMP_EXE_LINKER_FLAGS} )



# executables
add_executable (dvc ${CMAKE_CURRENT_SOURCE_DIR}/dvc.cpp )

target_link_libraries (dvc cildvc_static)
include_directories(dvc PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR} 
        $<BUILD_INTERFACE:${OpenMP_CXX_INCLUDE_DIRS}>
)

add_executable (strain ${CMAKE_CURRENT_SOURCE_DIR}/strain.cpp )

target_link_libraries (strain cildvc_static)
include_directories(strain PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR} 
    $<BUILD_INTERFACE:${OpenMP_CXX_INCLUDE_DIRS}>
)

if(BUILD_TEST)
  add_executable (tests ${CMAKE_CURRENT_SOURCE_DIR}/tests.cpp )
  #TARGET_LINK_LIBRARIES (tests cilDVC Eigen3::Eigen)
  target_link_libraries (tests cildvc_static)
  include_directories(tests PUBLIC
  	${CMAKE_CURRENT_SOURCE_DIR} )
endif()

# BKB: commenting out next 2 lines for make of Core to work
# add_executable(parse_header ${CMAKE_CURRENT_SOURCE_DIR}/npy/parse.cpp)
# set_property(TARGET parse_header PROPERTY CXX_STANDARD 11)

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

install(TARGETS dvc strain
	RUNTIME  DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
       )

if(BUILD_TEST)
  install(TARGETS tests
  RUNTIME  DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
  )
endif()

#install(TARGETS cildvc_shared
#	LIBRARY  DESTINATION "${INSTALL_LIB_DIR}" COMPONENT lib
#	PUBLIC_HEADER DESTINATION "${INSTALL_INCLUDE_DIR}" COMPONENT dev
#	RUNTIME  DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
#	ARCHIVE  DESTINATION "${INSTALL_LIB_DIR}" COMPONENT lib
#	)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/Core/wip
	DESTINATION ${INSTALL_BIN_DIR})
