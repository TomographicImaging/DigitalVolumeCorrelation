::   Copyright 2018 Oregon State University
::   Copyright 2018 United Kingdom Research and Innovation
::
::   This program is free software: you can redistribute it and/or modify
::   it under the terms of the GNU General Public License as published by
::   the Free Software Foundation, either version 3 of the License, or
::   (at your option) any later version.
::
::   This program is distributed in the hope that it will be useful,
::   but WITHOUT ANY WARRANTY; without even the implied warranty of
::   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
::   GNU General Public License for more details.
::
::   You should have received a copy of the GNU General Public License
::   along with this program.  If not, see <https://www.gnu.org/licenses/>.
::
::   Author(s): Edoardo Pasca (UKRI-STFC)
::              Laura Murgatroyd (UKRI-STFC)

mkdir "%SRC_DIR%\ccpi"
cd %SRC_DIR%/ccpi

:: issue cmake to create setup.py
cmake "%RECIPE_DIR%\.." -DBUILD_CUDA=OFF -DCMAKE_BUILD_TYPE="Release" -DLIBRARY_LIB="%CONDA_PREFIX%\lib" -DLIBRARY_INC="%CONDA_PREFIX%" -DCMAKE_INSTALL_PREFIX="%PREFIX%\Library" -DBUILD_TEST=ON

:: nmake install
:: if errorlevel 1 exit 1

:: For visual studio 2019:
:: issue cmake to create setup.py for windows
:: cmake "%RECIPE_DIR%\.."  -G "Visual Studio 16 2019" -A x64 -DBUILD_PYTHON_WRAPPERS=OFF -DCONDA_BUILD=ON -DBUILD_CUDA=OFF -DCMAKE_BUILD_TYPE="Release" -DLIBRARY_LIB="%CONDA_PREFIX%\lib" -DLIBRARY_INC="%CONDA_PREFIX%" -DCMAKE_INSTALL_PREFIX="%PREFIX%\Library" 
cmake --build . --target install --config Release

if errorlevel 1 exit 1

