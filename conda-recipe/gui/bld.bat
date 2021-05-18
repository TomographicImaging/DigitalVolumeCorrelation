::IF NOT DEFINED CIL_VERSION (
::ECHO CIL_VERSION Not Defined.
::exit 1
::)

mkdir "%SRC_DIR%"
copy "%RECIPE_DIR%\..\..\Apps\DVC_configurator\setup.py" "%SRC_DIR%\setup.py"
cd %SRC_DIR%

%PYTHON% setup.py install
if errorlevel 1 exit 1
