#   Copyright 2018 Oregon State University
#   Copyright 2018 United Kingdom Research and Innovation
#
#   This program is free software: you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program.  If not, see <https://www.gnu.org/licenses/>.
#
#   Author(s): Edoardo Pasca (UKRI-STFC),
#              Gemma Fardell (UKRI-STFC), 
#              Brian Bay (OSU)


if(APPLE)
  cmake_minimum_required(VERSION 3.16)
else()
  cmake_minimum_required(VERSION 3.1.3)
endif()

# get the current version and date
execute_process(COMMAND git describe 
                RESULT_VARIABLE worked 
                OUTPUT_VARIABLE dvc_version
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                OUTPUT_STRIP_TRAILING_WHITESPACE)
if (NOT worked EQUAL 0)
  message(FATAL_ERROR "git describe returned ${worked}")
endif()
execute_process(COMMAND git show -s --format=%cd --date=short 
                RESULT_VARIABLE worked 
                OUTPUT_VARIABLE rev_date
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                OUTPUT_STRIP_TRAILING_WHITESPACE)
if (NOT worked EQUAL 0)
  message(FATAL_ERROR "git describe returned ${worked}")
endif()

project(cildvc LANGUAGES CXX)

set (CMAKE_CXX_STANDARD 11)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_CXX_EXTENSIONS OFF)

option (CONDA_BUILD "Conda Build" OFF)
set (CMAKE_BUILD_TYPE "Release")

set(INSTALL_DIR "" CACHE PATH "Install Directory")
if (INSTALL_DIR)
    set(INSTALL_BIN_DIR "${INSTALL_DIR}")
else()
    set(INSTALL_BIN_DIR "${CMAKE_INSTALL_PREFIX}")
endif()
message(STATUS "DVC will be installed in " ${INSTALL_BIN_DIR})


# downloads Eigen 3.4
include(ExternalProject)
ExternalProject_Add(eigen
  # GIT_REPOSITORY    https://gitlab.com/libeigen/eigen.git
  # GIT_TAG           3.4.0
  GIT_REPOSITORY    https://github.com/vais-ral/Eigen3.git
  GIT_TAG           master
  SOURCE_DIR        ${CMAKE_SOURCE_DIR}/Core/eigen
  UPDATE_COMMAND    ""
  PATCH_COMMAND     ""
  CONFIGURE_COMMAND ""
  BUILD_COMMAND     ""
  INSTALL_COMMAND   ""
)

add_subdirectory(Core)
