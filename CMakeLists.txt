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
cmake_minimum_required(VERSION 3.0)
project(cildvc)

set (CMAKE_CXX_STANDARD 98)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_CXX_EXTENSIONS OFF)

option (BUILD_PYTHON_WRAPPER "Build Python Wrappers" OFF)
option (CONDA_BUILD "Conda Build" OFF)
set (CMAKE_BUILD_TYPE "Release")

set(INSTALL_DIR "" CACHE PATH "Install Directory")
if (INSTALL_DIR)
    set(INSTALL_BIN_DIR "${INSTALL_DIR}")
else() 
    set(INSTALL_BIN_DIR "${CMAKE_INSTALL_PREFIX}")
endif()
message(STATUS "DVC will be installed in " ${INSTALL_BIN_DIR})

add_subdirectory(Core)



if (BUILD_PYTHON_WRAPPER)
  add_subdirectory(Wrappers/Python)
endif()


