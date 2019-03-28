# CCPi-DVC
Digital Volume Correlation

| Master | Development | Anaconda binaries |
|--------|-------------|-------------------|
| [![Build Status](https://anvil.softeng-support.ac.uk/jenkins/buildStatus/icon?job=CILsingle/CCPi-DVC)](https://anvil.softeng-support.ac.uk/jenkins/job/CILsingle/job/CCPi-DVC/) | [![Build Status](https://anvil.softeng-support.ac.uk/jenkins/buildStatus/icon?job=CILsingle/CCPi-DVC-dev)](https://anvil.softeng-support.ac.uk/jenkins/job/CILsingle/job/CCPi-DVC-dev/) | ![conda version](https://anaconda.org/ccpi/ccpi-dvc/badges/version.svg) ![conda last release](https://anaconda.org/ccpi/ccpi-dvc/badges/latest_release_date.svg) [![conda platforms](https://anaconda.org/ccpi/ccpi-dvc/badges/platforms.svg) ![conda dowloads](https://anaconda.org/ccpi/ccpi-dvc/badges/downloads.svg)](https://anaconda.org/ccpi/ccpi-dvc) |

## Build

The package comes as a [CMake](https://cmake.org) project so you will need CMake (v.>=3) to configure it. Additionally you will need a C compiler, `make` (on linux). The toolkit comes with an executable `dvc` which may be used directly. We provide wrappers for Python.

1. Clone this repository to a directory, i.e. `CCPi-DVC`, 
2. create a build directory. 
3. Issue `cmake` to configure (or `cmake-gui`, or `ccmake`, or `cmake3`). Use additional flags to fine tune the configuration. 

### CMake flags
Flags used during configuration

| CMake flag | type | meaning |
|:---|:----|:----|
| `CMAKE_CXX_STANDARD` | string | Defaults to `98`. There are performance issues with `C++11`.
| `BUILD_PYTHON_WRAPPER` | bool | `ON\|OFF` whether to build the Python wrapper |
| `CMAKE_INSTALL_PREFIX` | path | your favourite install directory |
| `PYTHON_DEST_DIR` | path | python modules install directory (default `${CMAKE_INSTALL_PREFIX}/python`) |
| `CONDA_BUILD`| bool | `ON\|OFF` whether it is installed with `setup.py install`|
|`PYTHON_EXECUTABLE` | path | /path/to/python/executable|

Here an example of build on Linux:

```bash
git clone https://github.com/vais-ral/CCPi-DVC.git
mkdir build
cd build
cmake ../CCPi-Regularisation-Toolkit -DCONDA_BUILD=OFF -DBUILD_PYTHON_WRAPPER=ON -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=<your favourite install directory>
make install
```

### Python
#### Python binaries
Python binaries are distributed via the [ccpi](https://anaconda.org/ccpi/ccpi-dvc) conda channel. Currently we produce packages for Linux64, Python 2.7, 3.5 and 3.6, NumPy 1.12 and 1.13.

```
conda install ccpi-dvc -c ccpi -c conda-forge
```

#### Python (conda-build)
```
	conda build Wrappers/Python/conda-recipe --numpy 1.12 --python 3.5 
	conda install ccpi-dvc --use-local --force
```

#### Python build

If passed `CONDA_BUILD=ON` the software will be installed by issuing `python setup.py install` which will install in the system python (or whichever other python it's been picked up by CMake at configuration time.) 
If passed `CONDA_BUILD=OFF` the software will be installed in the directory pointed by `${PYTHON_DEST_DIR}` which defaults to `${CMAKE_INSTALL_PREFIX}/python`. Therefore this directory should be added to the `PYTHONPATH`.

If Python is not picked by CMake you can provide the additional flag to CMake `-DPYTHON_EXECUTABLE=/path/to/python/executable`.
