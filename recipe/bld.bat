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

mkdir "%SRC_DIR%\ccpi" 2>nul
cd /d "%SRC_DIR%\ccpi"

:: issue cmake to create setup.py
cmake -S "%RECIPE_DIR%\.." -G "Ninja" -DCMAKE_BUILD_TYPE="Release" -DLIBRARY_LIB="%CONDA_PREFIX%\lib" -DLIBRARY_INC="%CONDA_PREFIX%" -DCMAKE_INSTALL_PREFIX="%PREFIX%\Library" -DBUILD_TEST=ON

:: nmake install
:: if errorlevel 1 exit 1

:: For visual studio 2019:
:: issue cmake to create setup.py for windows
cmake --build . --target install --config Release -j
ctest --output-on-failure
if errorlevel 1 exit 1

