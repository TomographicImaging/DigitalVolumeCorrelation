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
CMAKE_MINIMUM_REQUIRED (VERSION 3.0)
PROJECT (CCPiDVC)

SET(CCPIDVC_MAJOR_VERSION 0)
SET(CCPIDVC_MINOR_VERSION 9)
SET(CCPIDVC_PATCH_VERSION 3)
SET(CCPIDVC_VERSION 
		${CCPIDVC_MAJOR_VERSION}.${CCPIDVC_MINOR_VERSION}.${CCPIDVC_PATCH_VERSION})
SET(INSTALL_LIB_DIR lib CACHE PATH "Installation directory for libraries")
SET(INSTALL_BIN_DIR bin CACHE PATH "Installation directory for executables")
SET(INSTALL_INCLUDE_DIR include CACHE PATH "Installation directory for header files")

IF(WIN32 AND NOT CYGWIN)
	SET(DEF_INSTALL_CMAKE_DIR cmake)
ELSE()
	SET(DEF_INSTALL_CMAKE_DIR lib/cmake/CCPiDVC)
ENDIF()

SET(INSTALL_CMAKE_DIR ${DEF_INSTALL_CMAKE_DIR} CACHE PATH
	"Installation directory for CMake files")
	
#Make relative paths absolute
FOREACH(p LIB BIN INCLUDE CMAKE)
	SET(var INSTALL_{p}_DIR)
	IF(NOT IS_ABSOLUTE "${${var}}")
		SET(${var} "${CMAKE_INSTALL_PREFIX}/${${var}}")
	ENDIF()
ENDFOREACH()

#setup include directories
INCLUDE_DIRECTORIES(
	"${PROJECT_SOURCE_DIR}"
"${PROJECT_BINARY_DIR}")

# Add sub-directories
ADD_SUBDIRECTORY(Core)

#OPTION(BUILD_AVIZO_PLUGIN "Build Avizo plugin" OFF)
#IF(BUILD_AVIZO_PLUGIN)
#	ADD_SUBDIRECTORY(Wrappers/Avizo)
#ENDIF()
#OPTION(BUILD_PARAVIEW_PLUGIN "Build Paraview plugin" OFF)
#IF(BUILD_PARAVIEW_PLUGIN)
#	ADD_SUBDIRECTORY(Wrappers/Paraview)
#ENDIF()
#OPTION(BUILD_IMAGEJ_PLUGIN "Build ImageJ plugin" OFF)
#IF(BUILD_IMAGEJ_PLUGIN)
#	add_subdirectory(Wrappers/ImageJ)
#ENDIF()

SET(CCPI_DVC_BUILD_LIBRARIES "cilDVC")
export(TARGETS cilDVC
	FILE "${PROJECT_BINARY_DIR}/CCPiDVCTargets.cmake")

export(PACKAGE CCPiDVC)
file(RELATIVE_PATH REL_INCLUDE_DIR "${INSTALL_CMAKE_DIR}"
	"${INSTALL_INCLUDE_DIR}")
# for build tree
set(CONF_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}" "${PROJECT_BINARY_DIR}")
configure_file(${CMAKE_SOURCE_DIR}/CMake/CCPiDVCConfig.cmake.in  
		"${PROJECT_BINARY_DIR}/CCPiDVCConfig.cmake" @ONLY)
# for install tree
set(CONF_INCLUDE_DIRS "\${CCPIDVC_CMAKE_DIR}/${REL_INCLUDE_DIR}")
configure_file(${CMAKE_SOURCE_DIR}/CMake/CCPiDVCConfig.cmake.in
		"${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CCPiDVCConfig.cmake" @ONLY)
# for both
configure_file(${CMAKE_SOURCE_DIR}/CMake/CCPiDVCConfigVersion.cmake.in
		"${PROJECT_BINARY_DIR}/CCPiDVCConfigVersion.cmake" @ONLY)

#Install
install(FILES
	"${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CCPiDVCConfig.cmake"
	"${PROJECT_BINARY_DIR}/CCPiDVCConfigVersion.cmake"
	DESTINATION "${INSTALL_CMAKE_DIR}" COMPONENT dev)

# Install the export set
install(EXPORT CCPiDVCTargets DESTINATION
"${INSTALL_CMAKE_DIR}" COMPONENT dev)