::   Copyright 2018 Oregon State University
::   Copyright 2018 United Kingdom Research and Innovation
::
::   Licensed under the Apache License, Version 2.0 (the "License");
::   you may not use this file except in compliance with the License.
::   You may obtain a copy of the License at
::
::       http://www.apache.org/licenses/LICENSE-2.0
::
::   Unless required by applicable law or agreed to in writing, software
::   distributed under the License is distributed on an "AS IS" BASIS,
::   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
::   See the License for the specific language governing permissions and
::   limitations under the License.
::
::   Author(s): Edoardo Pasca (UKRI-STFC)
::              Laura Murgatroyd (UKRI-STFC)

mkdir "%SRC_DIR%\ccpi"
cd %SRC_DIR%/ccpi

:: issue cmake to create setup.py
cmake "%RECIPE_DIR%\.." -DCONDA_BUILD=ON -DBUILD_CUDA=OFF -DCMAKE_BUILD_TYPE="Release" -DLIBRARY_LIB="%CONDA_PREFIX%\lib" -DLIBRARY_INC="%CONDA_PREFIX%" -DCMAKE_INSTALL_PREFIX="%PREFIX%\Library" 

:: nmake install
:: if errorlevel 1 exit 1

:: For visual studio 2019:
:: issue cmake to create setup.py for windows
:: cmake "%RECIPE_DIR%\.."  -G "Visual Studio 16 2019" -A x64 -DBUILD_PYTHON_WRAPPERS=OFF -DCONDA_BUILD=ON -DBUILD_CUDA=OFF -DCMAKE_BUILD_TYPE="Release" -DLIBRARY_LIB="%CONDA_PREFIX%\lib" -DLIBRARY_INC="%CONDA_PREFIX%" -DCMAKE_INSTALL_PREFIX="%PREFIX%\Library" 
cmake --build . --target install --config Release
if errorlevel 1 exit 1

