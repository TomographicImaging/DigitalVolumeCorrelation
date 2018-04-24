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
PROJECT (cilDVC)
FIND_PACKAGE (Eigen3 3.3 REQUIRED NO_MODULE)

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
	
IF(NOT WIN32)
   SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
ENDIF()
	
ADD_EXECUTABLE (dvc dvc.cpp ${DVC_SRCS})
TARGET_LINK_LIBRARIES (dvc Eigen3::Eigen)
INCLUDE_DIRECTORIES(dvc PUBLIC 
		      ${CMAKE_CURRENT_SOURCE_DIR}/ )
SET_TARGET_PROPERTIES(cilDVC PROPERTIES PUBLIC_HEADER "BoundBox.h;Cloud.h;DataCloud.h;FloatingCloud.h;InputRead.h;Interpolate.h;Matrix_4d.h;ObjectiveFunctions.h;Point.h;Search.h;SearchParams.h;Utility.h;dvc.h") 
			  
INSTALL(TARGETS cilDVC EXPORT CCPiDVCTargets
        LIBRARY  DESTINATION "${INSTALL_LIB_DIR}" COMPONENT lib
	PUBLIC_HEADER DESTINATION "${INSTALL_INCLUDE_DIR}" COMPONENT dev
	RUNTIME  DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
	ARCHIVE  DESTINATION "${INSTALL_LIB_DIR}" COMPONENT lib
       )


############Expose include files
set(${PROJECT_NAME}_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}
                                 ${C_INCLUDE_DIRS}
CACHE INTERNAL "${PROJECT_NAME}: Include Directories" FORCE)			  