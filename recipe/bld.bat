mkdir "%SRC_DIR%\ccpi"
ROBOCOPY /E "%RECIPE_DIR%\..\..\Wrappers\Python\src" "%SRC_DIR%\ccpi\src"
ROBOCOPY /E "%RECIPE_DIR%\..\..\Wrappers\Python\ccpi" "%SRC_DIR%\ccpi\ccpi"
cd %SRC_DIR%/ccpi


:: Original version of cmake command:
:: issue cmake to create setup.py
::cmake -G "NMake Makefiles" %RECIPE_DIR%\..\..\ -DBUILD_PYTHON_WRAPPERS=OFF -DCONDA_BUILD=ON -DBUILD_CUDA=OFF -DCMAKE_BUILD_TYPE="Release" -DLIBRARY_LIB="%CONDA_PREFIX%\lib" -DLIBRARY_INC="%CONDA_PREFIX%" -DCMAKE_INSTALL_PREFIX="%PREFIX%\Library" 

:: nmake install
:: if errorlevel 1 exit 1

:: For visual studio 2019:
:: issue cmake to create setup.py for windows
cmake "%RECIPE_DIR%\.."  -G "Visual Studio 16 2019" -A x64 -DBUILD_PYTHON_WRAPPERS=OFF -DCONDA_BUILD=ON -DBUILD_CUDA=OFF -DCMAKE_BUILD_TYPE="Release" -DLIBRARY_LIB="%CONDA_PREFIX%\lib" -DLIBRARY_INC="%CONDA_PREFIX%" -DCMAKE_INSTALL_PREFIX="%PREFIX%\Library" 
cmake --build . --target install 
if errorlevel 1 exit 1

