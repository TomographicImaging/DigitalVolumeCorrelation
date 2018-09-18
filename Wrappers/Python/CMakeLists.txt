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
find_package(PythonInterp REQUIRED)
if (PYTHONINTERP_FOUND)
  message ("Current Python " ${PYTHON_VERSION_STRING} " found " ${PYTHON_EXECUTABLE})
endif()
if (CONDA_BUILD)
else()
set(PYTHON_DEST_DIR "" CACHE PATH "Directory of the Matlab wrappers")
  if (PYTHON_DEST_DIR)
   set(PYTHON_DEST "${PYTHON_DEST_DIR}")
  else() 
    set(PYTHON_DEST "${CMAKE_INSTALL_PREFIX}/python")
  endif()
  message(STATUS "Python wrappers will be installed in " ${PYTHON_DEST})
endif()

find_package(PythonInterp REQUIRED)
find_package(PythonLibs)
if (PYTHONINTERP_FOUND)
  message(STATUS "Found PYTHON_EXECUTABLE=${PYTHON_EXECUTABLE}")
  message(STATUS "Python version ${PYTHON_VERSION_STRING}")
endif()
if (PYTHONLIBS_FOUND)
  message(STATUS "Found PYTHON_INCLUDE_DIRS=${PYTHON_INCLUDE_DIRS}")
  message(STATUS "Found PYTHON_LIBRARIES=${PYTHON_LIBRARIES}")
endif()

if (PYTHONINTERP_FOUND)
  message ("Current Python " ${PYTHON_VERSION_STRING} " found " ${PYTHON_EXECUTABLE})
  if (PYTHON_VERSION_MAJOR EQUAL "3")
    set (BOOST_PYTHON "python3")
	set (BOOST_NUMPY "numpy3")
  else()
    set (BOOST_PYTHON "python")
	set (BOOST_NUMPY "numpy")
  endif()
endif()

find_package(Boost
	COMPONENTS ${BOOST_PYTHON} ${BOOST_NUMPY})
if (Boost_FOUND)
else()
  #if (PYTHON_VERSION_MAJOR EQUAL "3")
  set (BOOST_PYTHON "python${PYTHON_VERSION_MAJOR}${PYTHON_VERSION_MINOR}")
  set (BOOST_NUMPY "numpy${PYTHON_VERSION_MAJOR}${PYTHON_VERSION_MINOR}")
endif()

find_package(Boost REQUIRED
	COMPONENTS ${BOOST_PYTHON} ${BOOST_NUMPY})

if (WIN32)
  list(GET Boost_LIBRARIES 0 place )
  get_filename_component(BOOST_PYTHON_LIB ${place} NAME_WE )
  list(GET Boost_LIBRARIES 1 place )
  get_filename_component(BOOST_NUMPY_LIB ${place} NAME_WE )
else()
  # on linux the library looks like libboost_numpy3.so:
  # we need to get rid of the lib at the beginning
  list(GET Boost_LIBRARIES 0 place )
  get_filename_component(place2 ${place} NAME_WE )
  string(REGEX REPLACE "^lib(.*)" "\\1" BOOST_PYTHON_LIB "${place2}")
  
  list(GET Boost_LIBRARIES 1 place )
  get_filename_component(place2 ${place} NAME_WE )
  string(REGEX REPLACE "^lib(.*)" "\\1" BOOST_NUMPY_LIB "${place2}")
endif()

message ("found " ${BOOST_PYTHON_LIB})
message ("found " ${BOOST_NUMPY_LIB})

message("Boost_INCLUDE_DIR ${Boost_INCLUDE_DIR}")
if (PYTHONINTERP_FOUND)
    message("Python found " ${PYTHON_EXECUTABLE})
    set(SETUP_PY_IN "${CMAKE_CURRENT_SOURCE_DIR}/setup-dvc.py.in")
    set(SETUP_PY    "${CMAKE_CURRENT_BINARY_DIR}/setup-dvc.py")
    #set(DEPS        "${CMAKE_CURRENT_SOURCE_DIR}/module/__init__.py")
    set (DEPS       "${CMAKE_BINARY_DIR}/Core/")
    set(OUTPUT      "${CMAKE_CURRENT_BINARY_DIR}/build/timestamp")

    if (WIN32)
      set (LIBRARY_LIB ${CMAKE_BINARY_DIR}/Core/${CMAKE_BUILD_TYPE})
    else()
      set (LIBRARY_LIB ${CMAKE_BINARY_DIR}/Core/)
    endif()
    configure_file(${SETUP_PY_IN} ${SETUP_PY})

    message("Core binary dir " ${CMAKE_BINARY_DIR}/Core/${CMAKE_BUILD_TYPE})
    
    if (CONDA_BUILD)
      add_custom_command(OUTPUT ${OUTPUT}
                       COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_BINARY_DIR}/src
                       COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/tomophantom ${CMAKE_CURRENT_BINARY_DIR}/tomophantom
                       COMMAND ${CMAKE_COMMAND} -E env CIL_VERSION=${CIL_VERSION}
                                                       PREFIX=${CMAKE_SOURCE_DIR}/Core 
                                                       LIBRARY_INC=${CMAKE_SOURCE_DIR}/Core 
                                                       LIBRARY_LIB=${CMAKE_BINARY_DIR}/Core
                                                       ${PYTHON_EXECUTABLE} ${SETUP_PY} install
                       COMMAND ${CMAKE_COMMAND} -E touch ${OUTPUT}
                       DEPENDS  cildvc_static)
      add_custom_target(PythonWrapper ALL DEPENDS ${OUTPUT})

    else()
            
	  message("Library should be in ${LIBRARY_LIB}")
      add_custom_command(OUTPUT ${OUTPUT}
                       COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_BINARY_DIR}/src
                       COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/ccpi ${CMAKE_CURRENT_BINARY_DIR}/ccpi
                       COMMAND ${CMAKE_COMMAND} -E env CIL_VERSION=${CIL_VERSION}
                                                       PREFIX=${CMAKE_SOURCE_DIR}/Core 
                                                       LIBRARY_INC=${CMAKE_SOURCE_DIR}/Core 
                                                       LIBRARY_LIB=${CMAKE_BINARY_DIR}/Core/${CMAKE_BUILD_TYPE}
                                                       ${PYTHON_EXECUTABLE} ${SETUP_PY} build_ext --inplace
                       COMMAND ${CMAKE_COMMAND} -E touch ${OUTPUT}
                       DEPENDS  cildvc_static
                       )
      add_custom_target(PythonWrapper ALL DEPENDS ${OUTPUT})
      install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/ccpi 
              DESTINATION ${PYTHON_DEST})
    endif()
    
    
    

    #install(CODE "execute_process(COMMAND ${PYTHON} ${SETUP_PY} install)")
endif()
